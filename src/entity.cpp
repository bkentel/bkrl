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
    explicit entity_parser(string_ref const filename) {
        auto const value = json::common::from_file(filename);
        rule_root(value);
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
        entity_.id = json::require_string(value[json::common::field_id]);

        auto const hash = entity_.id.hash;
        entities_.emplace(hash, std::move(entity_));
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
    explicit entity_locale_parser(string_ref const filename)
      : locale {filename}
    {
        if (string_type != json::common::filetype_entity) {
            BK_TODO_FAIL();
        }
        
        rule_root(root);
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


entity_def::definition_t
entity_def::load_definitions(string_ref const filename) {
    return entity_def::definition_t {entity_parser {filename}};
}

entity_def::localized_t
entity_def::load_localized_strings(string_ref const filename) {
    return entity_def::localized_t {entity_locale_parser {filename}};
}
