#include "messages.hpp"
#include "json.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

class bkrl::message_map::impl_t {
public:
    using cref = json::cref;

    //--------------------------------------------------------------------------
    void load(json::cref data) {
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    void rule_root(cref value) {
        rule_file_type(value);
        rule_language(value);
        rule_definitions(value);
    }
    
    //--------------------------------------------------------------------------
    void rule_file_type(cref value) {
        jc::get_filetype(value, jc::filetype_locale);
    }
       
    //--------------------------------------------------------------------------
    void rule_language(cref value) {
        auto const locale = jc::get_locale(value, jc::filetype_messages);
        if (!locale) {
            BK_TODO_FAIL();
        }

        cur_lang_ = *locale;
    }
    
    //--------------------------------------------------------------------------
    void rule_definitions(cref value) {
        auto const defs = json::require_array(value[jc::field_definitions]);
        cur_loc_.reserve(enum_value(message_type::enum_size));

        for (auto const& def : defs.array_items()) {
            rule_definition(def);
        }

        locales_.emplace(cur_lang_, std::move(cur_loc_));
    }
    
    //--------------------------------------------------------------------------
    void rule_definition(cref value) {
        auto const def = json::require_array(value, 2, 2);
        
        auto const id  = json::require_string(def[0]);
        auto const str = json::require_string(def[1]);

        auto const hash   = slash_hash32(id);
        auto const mapped = enum_map<message_type>::get(hash);

        if (mapped.value == message_type::invalid) {
            BK_TODO_FAIL();
        } else if (mapped.string != id) {
            BK_TODO_FAIL();
        }

        cur_loc_.emplace(mapped.value, str.to_string());
    }

    //--------------------------------------------------------------------------
    string_ref operator[](message_type const msg) const {
        static string_ref const undefined {"<undefined message>"};

        auto const it = current_locale_->find(msg);
        if (it == std::cend(*current_locale_)) {
            return undefined;
        }

        return it->second;
    }

    //--------------------------------------------------------------------------
    void set_locale(lang_id const lang) {
        auto const it = locales_.find(lang);
        if (it == std::end(locales_)) {
            BK_TODO_FAIL();
        }

        current_locale_ = &it->second;
    }
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;
    using locale_map = map_t<message_type, utf8string>;
    
    lang_id    cur_lang_;
    locale_map cur_loc_;

    map_t<lang_id, locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// message_map
////////////////////////////////////////////////////////////////////////////////
bkrl::message_map::~message_map() = default;

//------------------------------------------------------------------------------
bkrl::message_map::message_map()
  : impl_ {std::make_unique<impl_t>()}
{
}

//------------------------------------------------------------------------------
void
bkrl::message_map::load(json::cref value) {
    impl_->load(value);
}

//------------------------------------------------------------------------------
bkrl::string_ref
bkrl::message_map::operator[](message_type const msg) const {
    return (*impl_)[msg];
}

//------------------------------------------------------------------------------
void
bkrl::message_map::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}
