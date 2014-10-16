#include "entity.hpp"
#include "json.hpp"

using namespace bkrl;

namespace {

//==============================================================================
//==============================================================================
class entity_parser {
public:
    using cref = json::cref;

    //--------------------------------------------------------------------------
    explicit entity_parser(cref data) {
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    void rule_root(cref value) {
        json::require_object(value);
        json::common::get_filetype(value, json::common::filetype_entity);
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
        rule_items(value);
        rule_color(value);
        rule_tile(value);

        entity_.id = json::require_string(value[json::common::field_id]);

        auto const hash = entity_.id.hash;
        entities_.emplace(hash, std::move(entity_));
    }

    //--------------------------------------------------------------------------
    void rule_tile(cref value) {
        static utf8string const field {"tile"};

        //TODO should be able to clean this up a bit more

        auto array = json::require_array(value[field], 2, 2);
        
        entity_.tile_x = json::require_int<int16_t>(array[0]);
        entity_.tile_y = json::require_int<int16_t>(array[1]);

        if (entity_.tile_x < 0 || entity_.tile_y < 0) {
            BK_TODO_FAIL();
        }
    }


    //--------------------------------------------------------------------------
    void rule_color(cref value) {
        static utf8string const field {"color"};

        //TODO should be able to clean this up a bit more

        auto array = json::require_array(value[field], 3, 3);
        
        entity_.r = json::require_int<uint8_t>(array[0]);
        entity_.g = json::require_int<uint8_t>(array[1]);
        entity_.b = json::require_int<uint8_t>(array[2]);
    }


    //--------------------------------------------------------------------------
    void rule_items(cref value) {
        static utf8string const field {"items"};

        //TODO should be able to clean this up a bit more

        auto array = json::require_array(value[field], 2, 2);
        
        entity_.items.lo = json::require_int<int16_t>(array[0]);
        entity_.items.hi = json::require_int<int16_t>(array[1]);

        if (!entity_.items) {
            BK_TODO_FAIL();
        }
    }
    //--------------------------------------------------------------------------
    operator entity_def::definition_t::map_t&&() && {
        return std::move(entities_);
    }
private:
    entity_def entity_;

    entity_def::definition_t::map_t entities_;
};

//==============================================================================
//==============================================================================
class entity_locale_parser : public json::common::locale {
public:
    //--------------------------------------------------------------------------
    explicit entity_locale_parser(cref data)
      : locale {data}
    {
        if (string_type != json::common::filetype_entity) {
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

        locale_.name = name.get_value_or("<undefined>").to_string();
        locale_.text = text.get_value_or("<undefined>").to_string();

        auto const hash = slash_hash32(id);
        strings_.emplace(hash, std::move(locale_));
    }

    //--------------------------------------------------------------------------
    operator entity_def::localized_t::map_t&&() && {
        return std::move(strings_);
    }
private:
    entity_def::locale             locale_;
    entity_def::localized_t::map_t strings_;
};

} //namespace

entity_def::definition_t bkrl::load_entities(json::cref data) {
    return entity_def::definition_t {entity_parser {data}};
}

entity_def::localized_t bkrl::load_entities_locale(json::cref data) {
    return entity_def::localized_t {entity_locale_parser {data}};
}
