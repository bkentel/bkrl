#include "item.hpp"
#include "json.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

//==============================================================================
//!
//==============================================================================
class bkrl::detail::item_definitions_impl {
public:
    using definition = item_definitions::definition;
    using locale     = item_definitions::locale;
    using cref = bkrl::json::cref;  

    item_definitions_impl() {
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_definitions(cref data) {
        rule_def_root(data);
    }
    
    void rule_def_root(cref value) {
        json::require_object(value);

        rule_def_filetype(value);
        rule_def_definitions(value);
    }

    void rule_def_filetype(cref value) {
        jc::get_filetype(value, jc::filetype_item);
    }

    void rule_def_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        definitions_.reserve(array.size());
        
        for (auto&& def : array) {
            rule_def_definition(def);
        }
    }

    void rule_def_definition(cref value) {
        rule_def_id(value);
        rule_def_stack(value);
        rule_def_damage(value);

        definitions_.emplace(cur_def_.id.hash, std::move(cur_def_));
    }

    void rule_def_id(cref value) {
        auto const str = json::require_string(value[jc::field_id]);
        if (str.length() < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.id = str;
    }

    void rule_def_stack(cref value) {
        auto const stack = json::default_int(value[jc::field_stack], 1);
        if (stack < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.max_stack = stack;
    }

    void rule_def_damage(cref value) {
        using dist_t = item_definitions::dist_t;

        auto const dmg_min = value[jc::field_damage_min];
        auto const dmg_max = value[jc::field_damage_max];

        auto const no_min = dmg_min.is_null();
        auto const no_max = dmg_max.is_null();

        if (no_min ^ no_max) {
            BK_TODO_FAIL();
        }

        auto const has_damage = !no_min && !no_max;

        cur_def_.damage_min = has_damage ? jc::get_random(dmg_min) : dist_t {};
        cur_def_.damage_max = has_damage ? jc::get_random(dmg_max) : dist_t {};
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_locale(cref data) {
        rule_loc_root(data);
    }

    void rule_loc_root(cref value) {
        auto const locale = jc::get_locale(value, jc::filetype_item);
        if (!locale) {
            BK_TODO_FAIL();
        }
        
        cur_lang_ = *locale;

        rule_loc_definitions(value);
    }

    void rule_loc_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        cur_loc_map_.reserve(array.size());

        for (auto&& def : array) {
            rule_loc_definition(def);
        }

        locales_.emplace(cur_lang_, std::move(cur_loc_map_));
    }

    void rule_loc_definition(cref value) {
        static string_ref const undefined {"<undefined string>"};

        hashed_string_ref const id = json::require_string(value[jc::field_id]);
        
        assign(cur_loc_.name, json::default_string(value[jc::field_name], undefined));
        assign(cur_loc_.text, json::default_string(value[jc::field_text], undefined));
        assign(cur_loc_.sort, json::default_string(value[jc::field_sort], ""));

        cur_loc_map_.emplace(id.hash, std::move(cur_loc_));
    }

    ////////////////////////////////////////////////////////////////////////////
    definition const& get_definition(identifier const id) const {
        auto const it = definitions_.find(id.hash);
        if (it == std::end(definitions_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
    
    locale const& get_locale(identifier const id) const {
        auto const it = current_locale_->find(id.hash);
        if (it == std::end(*current_locale_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }

    void set_locale(lang_id const lang) {
        auto const it = locales_.find(lang);
        if (it == std::end(locales_)) {
            BK_TODO_FAIL();
        }

        current_locale_ = &it->second;
    }

    int definitions_size() const {
        return static_cast<int>(definitions_.size());
    }
    
    definition const& get_definition_at(int const index) const {
        BK_ASSERT(index < definitions_size());

        auto it = definitions_.begin();
        std::advance(it, index);
        return it->second;
    }
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    using locale_map = map_t<hash_t, locale>;

    item_definitions::definition cur_def_;
    item_definitions::locale     cur_loc_;
    lang_id                      cur_lang_;
    locale_map                   cur_loc_map_;

    map_t<hash_t,  definition> definitions_;
    map_t<lang_id, locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// item_definitions
////////////////////////////////////////////////////////////////////////////////
using bkrl::item_definitions;

//------------------------------------------------------------------------------
item_definitions::~item_definitions() = default;

//------------------------------------------------------------------------------
item_definitions::item_definitions()
  : impl_ {std::make_unique<detail::item_definitions_impl>()}
{
}

//------------------------------------------------------------------------------
void
bkrl::item_definitions::load_definitions(json::cref data) {
    impl_->load_definitions(data);
}

//------------------------------------------------------------------------------
void
bkrl::item_definitions::load_locale(json::cref data) {
    impl_->load_locale(data);
}

//------------------------------------------------------------------------------
item_definitions::definition const&
bkrl::item_definitions::get_definition(identifier const id) const {
    return impl_->get_definition(id);
}

//------------------------------------------------------------------------------
item_definitions::locale const&
bkrl::item_definitions::get_locale(identifier const id) const {
    return impl_->get_locale(id);
}

//------------------------------------------------------------------------------
void bkrl::item_definitions::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}

//------------------------------------------------------------------------------
int item_definitions::definitions_size() const {
    return impl_->definitions_size();
}

//------------------------------------------------------------------------------
item_definitions::definition const&
item_definitions::get_definition_at(int const index) const {
    return impl_->get_definition_at(index);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bkrl::item
bkrl::generate_item(
    random::generator&      gen
  , item_definitions const& defs
  , loot_table       const& table
) {
    auto const size = defs.definitions_size();
    auto const i    = random::uniform_range(gen, 0, size - 1);

    auto const& idef = defs.get_definition_at(i);

    item result;

    result.id = idef.id;

    auto const has_dmg_min = !!idef.damage_min;
    auto const has_dmg_max = !!idef.damage_max;

    BK_ASSERT_DBG(
         (has_dmg_min &&  has_dmg_max)
     || (!has_dmg_min && !has_dmg_max)
    );

    if (has_dmg_min && has_dmg_max) {
        result.damage_min = idef.damage_min(gen);
        result.damage_max = idef.damage_max(gen);
    }

    return result;
}
