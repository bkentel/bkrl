#include "item.hpp"
#include "json.hpp"

using namespace bkrl;

//TODO make atomic
bkrl::item::localized_t const* bkrl::item::current_locale = nullptr;

item_def::locale const& item_def::undefined() {
    static locale const value {"<undefined name>", "<undefined sort>", "<undefined text>"};
    return value;
}

namespace {

//==============================================================================
//!
//==============================================================================
class item_parser {
public:
    using cref = json::cref;

    //--------------------------------------------------------------------------
    explicit item_parser(cref data) {
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    explicit item_parser(path_string_ref const filename)
      : item_parser {json::common::from_file(filename)}
    {
    }

    //--------------------------------------------------------------------------
    void rule_root(cref value) {
        json::require_object(value);
        json::common::get_filetype(value, json::common::filetype_item);
        rule_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_definitions(cref value) {
        auto const definitions = json::require_array(value[json::common::field_definitions]);

        for (auto&& def : definitions.array_items()) {
            rule_definition(def);
        }
    }

    //--------------------------------------------------------------------------
    void rule_definition(cref value) {
        item_.id = json::require_string(value[json::common::field_id]);

        rule_stack(value);
        rule_damage(value);

        auto const hash = item_.id.hash;
        items_.emplace(hash, std::move(item_));
    }

    //--------------------------------------------------------------------------
    void rule_stack(cref value) {
        static utf8string const field {"stack"};

        item_.stack = json::optional_int(value[field]).get_value_or(1);
    }

    //--------------------------------------------------------------------------
    void rule_damage(cref value) {
        static utf8string const field_min {"damage_min"};
        static utf8string const field_max {"damage_max"};

        auto const dmg_min = value[field_min];
        auto const dmg_max = value[field_max];

        if (dmg_min.is_null() && dmg_max.is_null()) {
            item_.damage_min.clear();
            item_.damage_max.clear();
            return;
        }

        if (dmg_min.is_null() ^ dmg_max.is_null()) {
            BK_TODO_FAIL();
        }

        item_.damage_min = json::common::random {dmg_min};
        item_.damage_max = json::common::random {dmg_max};
    }

    //--------------------------------------------------------------------------
    operator item_def::definition_t::map_t&&() && {
        return std::move(items_);
    }
private:
    item_def item_;

    item_def::definition_t::map_t items_;
};

//==============================================================================
//!
//==============================================================================
class item_locale_parser : public json::common::locale {
public:
    //--------------------------------------------------------------------------
    explicit item_locale_parser(path_string_ref const filename)
      : item_locale_parser {json::common::from_file(filename)}
    {
    }

    //--------------------------------------------------------------------------
    explicit item_locale_parser(cref data)
      : locale {data}
    {
        if (string_type != json::common::filetype_item) {
            BK_TODO_FAIL();
        }
        
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    void rule_root(cref value) {
        rule_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_definitions(cref value) {
        auto const defs = definitions(value);

        for (auto&& def : defs.array_items()) {
            rule_definition(def);
        }
    }

    //--------------------------------------------------------------------------
    void rule_definition(cref value) {
        auto const id = json::require_string(value[json::common::field_id]);
        
        auto name = json::optional_string(value[json::common::field_name]);
        auto text = json::optional_string(value[json::common::field_text]);
        auto sort = json::optional_string(value[json::common::field_sort]);

        locale_.name = name.get_value_or("<undefined>").to_string();
        locale_.text = text.get_value_or("<undefined>").to_string();
        locale_.sort = sort.get_value_or(locale_.name).to_string();

        auto const hash = slash_hash32(id);
        strings_.emplace(hash, std::move(locale_));
    }

    //--------------------------------------------------------------------------
    operator item_def::localized_t::map_t&&() && {
        return std::move(strings_);
    }
private:
    item_def::locale             locale_;
    item_def::localized_t::map_t strings_;
};

} //namespace

item_def::definition_t bkrl::load_items(json::cref data) {
    return item_def::definition_t { item_parser {data} };
}

item_def::localized_t bkrl::load_items_locale(json::cref data) {
    return item_def::localized_t { item_locale_parser {data} };
}
