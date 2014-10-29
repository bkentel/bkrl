#include "entity.hpp"
#include "json.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

////////////////////////////////////////////////////////////////////////////////
// item_definitions
////////////////////////////////////////////////////////////////////////////////
struct bkrl::entity_definition {
    using dist_t = random::random_dist;

    static path_string tile_filename;
    static ipoint2     tile_size;

    entity_def_id id;      //!< entity id
    utf8string    id_string;
    dist_t        items;   //!< number of carried items
    int16_t       tile_x;  //!< tile x index
    int16_t       tile_y;  //!< tile y index
    uint8_t       r, g, b; //!< color
    dist_t        health;  //!< health
};

bkrl::path_string bkrl::entity_definition::tile_filename {};
bkrl::ipoint2     bkrl::entity_definition::tile_size     {0, 0};

//==============================================================================
//!
//==============================================================================
class bkrl::detail::entity_definitions_impl {
public:
    using definition = entity_definition;
    using locale     = entity_locale;
    using cref = bkrl::json::cref;  

    static utf8string const undefined_name;
    static utf8string const undefined_text;
    static locale     const undefined_locale;

    ////////////////////////////////////////////////////////////////////////////
    entity_definitions_impl() {
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_definitions(cref data) {
        rule_ent_root(data);
    }
    
    //--------------------------------------------------------------------------
    void rule_ent_root(cref value) {
        jc::get_filetype(value, jc::filetype_entity);

        rule_ent_tiles(value);
        rule_ent_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_ent_tiles(cref value) {
        definition::tile_filename = json::common::get_filename(value); //TODO

        auto const size = json::require_array(value[jc::field_tile_size], 2, 2);
        definition::tile_size.x = json::require_int(size[0]);
        definition::tile_size.y = json::require_int(size[1]);
    }

    //--------------------------------------------------------------------------
    void rule_ent_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        definitions_.reserve(array.size());

        for (auto&& def : array) {
            rule_ent_definition(def);
        }
    }

    //--------------------------------------------------------------------------
    void rule_ent_definition(cref value) {
        rule_end_id(value);
        rule_ent_items(value);
        rule_ent_color(value);
        rule_ent_tile(value);
        rule_ent_health(value);

        definitions_.emplace(cur_def_.id, std::move(cur_def_));
    }

    void rule_end_id(cref value) {
        auto const str = json::require_string(value[jc::field_id]);
        if (str.length() < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.id = entity_def_id {slash_hash32(str)};
        cur_def_.id_string = str.to_string();
    }

    //--------------------------------------------------------------------------
    void rule_ent_tile(cref value) {
        auto const tile = json::require_array(value[jc::field_tile], 2, 2);
        
        cur_def_.tile_x = json::require_int<int16_t>(tile[0]);
        cur_def_.tile_y = json::require_int<int16_t>(tile[1]);

        if (cur_def_.tile_x < 0 || cur_def_.tile_y < 0) {
            BK_TODO_FAIL();
        }
    }

    //--------------------------------------------------------------------------
    void rule_ent_color(cref value) {
        auto const color = json::require_array(value[jc::field_color], 3, 3);
        
        cur_def_.r = json::require_int<uint8_t>(color[0]);
        cur_def_.g = json::require_int<uint8_t>(color[1]);
        cur_def_.b = json::require_int<uint8_t>(color[2]);
    }

    //--------------------------------------------------------------------------
    void rule_ent_items(cref value) {
        cur_def_.items = jc::get_random(value[jc::field_items]);
    }

    //--------------------------------------------------------------------------
    void rule_ent_health(cref value) {
        cur_def_.health = jc::get_random(value[jc::field_health]);
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_locale(cref data) {
        rule_loc_root(data);
    }

    //--------------------------------------------------------------------------
    void rule_loc_root(cref value) {
        auto const locale = jc::get_locale(value, jc::filetype_entity);
        if (!locale) {
            BK_TODO_FAIL();
        }
        
        cur_lang_ = *locale;

        rule_loc_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_loc_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        cur_loc_map_.reserve(array.size());

        for (auto&& def : array) {
            rule_loc_definition(def);
        }

        locales_.emplace(cur_lang_, std::move(cur_loc_map_));
    }

    //--------------------------------------------------------------------------
    void rule_loc_definition(cref value) {
        auto const& str = json::require_string(value[jc::field_id]);
        auto const  id  = slash_hash32(str);

        assign(cur_loc_.name, json::default_string(value[jc::field_name], undefined_name));
        assign(cur_loc_.text, json::default_string(value[jc::field_text], undefined_text));

        cur_loc_map_.emplace(id, std::move(cur_loc_));
    }

    ////////////////////////////////////////////////////////////////////////////
    definition const& get_definition(entity_def_id const id) const {
        auto const it = definitions_.find(id);
        if (it == std::end(definitions_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
    
    locale const& get_locale(entity_def_id const id) const {
        auto const it = current_locale_->find(id);
        if (it == std::end(*current_locale_)) {
            return undefined_locale;
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

    auto get_definitions_size() const {
        return static_cast<int>(definitions_.size());
    }
    
    auto const& get_definition_at(int const index) const {
        BK_ASSERT(index < get_definitions_size());

        auto it = definitions_.begin();
        std::advance(it, index);
        return it->second;
    }
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    using locale_map = map_t<entity_def_id, locale>;

    definition cur_def_;
    locale     cur_loc_;
    lang_id    cur_lang_;
    locale_map cur_loc_map_;

    map_t<entity_def_id, definition> definitions_;
    map_t<lang_id,       locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

bkrl::utf8string const bkrl::detail::entity_definitions_impl::undefined_name {"{undefined name}"};
bkrl::utf8string const bkrl::detail::entity_definitions_impl::undefined_text {"{undefined text}"};

bkrl::entity_locale const bkrl::detail::entity_definitions_impl::undefined_locale {
    undefined_name
  , undefined_text
};

////////////////////////////////////////////////////////////////////////////////
// entity
////////////////////////////////////////////////////////////////////////////////
bkrl::entity_render_info_t
bkrl::entity::render_info(entity_definitions const& defs) const {
    auto const& def = defs.get_definition(id);
    
    return entity_render_info_t {
        def.tile_x, def.tile_y
      , def.r, def.g, def.b, 255 //TODO
    };
}

bkrl::string_ref bkrl::entity::name(defs_t defs) const {
    return defs.get_locale(id).name;
}

bool bkrl::entity::apply_damage(damage_t const delta) {
    return (data.cur_health -= delta) <= 0;
}
////////////////////////////////////////////////////////////////////////////////
// entity_definitions
////////////////////////////////////////////////////////////////////////////////
bkrl::path_string const& bkrl::entity_definitions::tile_filename() {
    return entity_definition::tile_filename;
}

bkrl::ipoint2 bkrl::entity_definitions::tile_size() {
    return entity_definition::tile_size;
}

//------------------------------------------------------------------------------
bkrl::entity_definitions::~entity_definitions() = default;

//------------------------------------------------------------------------------
bkrl::entity_definitions::entity_definitions()
  : impl_ {std::make_unique<detail::entity_definitions_impl>()}
{
}

//------------------------------------------------------------------------------
void bkrl::entity_definitions::load_definitions(json::cref data) {
    impl_->load_definitions(data);
}

//------------------------------------------------------------------------------
void bkrl::entity_definitions::load_locale(json::cref data) {
    impl_->load_locale(data);
}

//------------------------------------------------------------------------------
bkrl::entity_definition const&
bkrl::entity_definitions::get_definition(entity_def_id const id) const {
    return impl_->get_definition(id);
}

//------------------------------------------------------------------------------
bkrl::entity_locale const&
bkrl::entity_definitions::get_locale(entity_def_id const id) const {
    return impl_->get_locale(id);
}

//------------------------------------------------------------------------------
void bkrl::entity_definitions::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}

//------------------------------------------------------------------------------
int bkrl::entity_definitions::get_definitions_size() const {
    return impl_->get_definitions_size();
}

//------------------------------------------------------------------------------
bkrl::entity_definition const&
bkrl::entity_definitions::get_definition_at(int const index) const {
    return impl_->get_definition_at(index);
}

////////////////////////////////////////////////////////////////////////////////
// generate_entity
////////////////////////////////////////////////////////////////////////////////
bkrl::entity
bkrl::generate_entity(
    random::generator&        gen
  , entity_definitions const& entity_defs
  , item_definitions   const& item_defs
  , item_store&               items
  , spawn_table        const& table
) {
    auto const size  = entity_defs.get_definitions_size();
    auto const index = random::uniform_range(gen, 0, size - 1);

    auto const& def = entity_defs.get_definition_at(index);
    auto const& id  = def.id;

    entity result;

    result.id = id;
    result.data.max_health = static_cast<health_t>(def.health(gen));
    result.data.cur_health = result.data.max_health;
    result.data.position = {0, 0};

    auto const item_count = def.items(gen);

    if (item_count == 0) {
        return result;
    }

    item_birthplace origin;
    origin.type = item_birthplace::entity;
    origin.id   = id_to_value(id);

    auto const ltable = loot_table {};

    for (int i = 0; i < item_count; ++i) {
        result.data.items.insert(
            generate_item(gen, items, item_defs, ltable, origin)
          , item_defs
          , items
        );
    }

    return result;
}
