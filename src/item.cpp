#include "item.hpp"
#include "json.hpp"

using namespace bkrl;

namespace {

struct common_rules {
    static void match_file_type(json::cref value, string_id const expected) {
        static utf8string const field {"file_type"};
        string_id const actual = json::require_string(value[field]);

        if (actual != expected) {
            BK_TODO_FAIL();
        }
    }

    static void match_lang_string_type(json::cref value, string_id const expected) {
        static utf8string const field {"string_type"};
        string_id const actual = json::require_string(value[field]);

        if (actual != expected) {
            BK_TODO_FAIL();
        }
    }

    static json::cref rule_definitions(json::cref value) {
        static utf8string const field {"definitions"};
        return json::require_array(value[field]);
    }

    static string_id rule_id(json::cref value) {
        static utf8string const field {"id"};
        return json::require_string(value[field]);
    }

    static string_id language_id(json::cref value) {
        static utf8string const field {"language"};
        return json::require_string(value[field]);
    }
};

class item_material_parser {
public:
    using cref = json::cref;

    explicit item_material_parser(utf8string const& data) {
        auto const root = json::common::from_memory(data);
        rule_root(root);
    }

    void add_definition() {
        auto const key = material_.id.hash;
        auto const it = result_.emplace(key, std::move(material_));
        if (!it.second) {
            BK_TODO_FAIL();
        }
    }

    void rule_root(cref value) {
        json::require_object(value);

        rule_file_type(value);
        rule_definitions(value);
    }

    void rule_file_type(cref value) {
        static string_id const expected {"MATERIAL"};
        common_rules::match_file_type(value, expected);
    }

    void rule_definitions(cref value) {
        auto const defs = common_rules::rule_definitions(value);

        for (auto const& def : defs.array_items()) {
            rule_definition(def);
        }
    }

    void rule_definition(cref value) {
        json::require_object(value);

        rule_def_id(value);
        rule_def_color(value);
        rule_def_weight_mod(value);
        rule_def_value_mod(value);
        rule_def_tags(value);

        add_definition();
    }

    void rule_def_id(cref value) {
        material_.id = common_rules::rule_id(value);
    }

    void rule_def_color(cref value) {
        static utf8string const field {"color"};
        material_.color = json::require_string(value[field]);
    }
    
    void rule_def_weight_mod(cref value) {
        static utf8string const field {"weight_mod"};
        material_.weight_mod = json::require_float<>(value, field, 0.0f, 100.0f);
    }

    void rule_def_value_mod(cref value) {
        static utf8string const field {"value_mod"};
        material_.value_mod = json::require_float<>(value, field, 0.0f, 100.0f);
    }

    void rule_def_tags(cref value) {
        static utf8string const field {"tags"};
        auto const tags = json::require_array(value[field]);

        for (auto const& tag : tags.array_items()) {
            rule_def_tag(tag);
        }
    }

    void rule_def_tag(cref value) {
        material_.tags.emplace_back(json::require_string(value));
    }

    definition<item_material_def> to_definition() {
        return definition<item_material_def>{std::move(result_)};
    }
private:
    item_material_def material_;
    definition<item_material_def>::map_t result_;
};

class item_material_locale_parser {
public:
    using cref = json::cref;

    explicit item_material_locale_parser(utf8string const& data) {
        std::string error;
        auto const root = json11::Json::parse(data, error);
        if (!error.empty()) {
            BK_TODO_FAIL();
        }

        rule_root(root);
    }

    void rule_root(cref value) {
        json::require_object(value);

        rule_file_type(value);
        rule_string_type(value);
        rule_language_id(value);
        rule_definitions(value);
    }

    void rule_file_type(cref value) {
        static string_id const expected {"LOCALE"};
        common_rules::match_file_type(value, expected);
    }

    void rule_string_type(cref value) {
        static string_id const expected {"MATERIAL"};
        common_rules::match_lang_string_type(value, expected);
    }

    void rule_language_id(cref value) {
        language_ = common_rules::language_id(value);
    }

    void rule_definitions(cref value) {
        auto const defs = common_rules::rule_definitions(value);

        for (auto const& def : defs.array_items()) {
            rule_definition(def);
        }
    }

    void rule_definition(cref value) {
        json::require_object(value);

        rule_def_id(value);
        rule_def_name(value);
        rule_def_text(value);

        add_item();
    }

    void rule_def_id(cref value) {
        static utf8string const field {"id"};
        id_ = json::require_string(value[field]);
    }

    void rule_def_name(cref value) {
        static utf8string const field {"name"};
        locale_.name = json::require_string(value[field]).to_string();
    }

    void rule_def_text(cref value) {
        static utf8string const field {"text"};
        locale_.text = json::require_string(value[field]).to_string();
    }

    void add_item() {
        //auto lang_it = result_.find(language_.hash);
        //if (lang_it == std::end(result_)) {
        //    auto const result = result_.emplace(
        //        language_.hash, localized_string<item_material_def>::map_t::mapped_type {}
        //    );

        //    if (!result.second) {
        //        BK_TODO_FAIL();
        //    }

        //    lang_it = result.first;
        //}

        //auto const result = lang_it->second.emplace(id_.hash, std::move(locale_));
        //if (!result.second) {
        //    BK_TODO_FAIL();
        //}
    }

    localized_string<item_material_def> to_localized_string() {
        return localized_string<item_material_def>{std::move(result_)};
    }
private:
    string_id language_;
    string_id id_;

    item_material_def::locale locale_;
    localized_string<item_material_def>::map_t result_;
};

//==============================================================================
//==============================================================================
class item_parser {
public:
    using cref = json::cref;

    //--------------------------------------------------------------------------
    explicit item_parser(string_ref const filename) {
        auto const value = json::common::from_file(filename);
        rule_root(value);
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
        rule_stack(value);
        item_.id = json::require_string(value[json::common::field_id]);

        auto const hash = item_.id.hash;
        items_.emplace(hash, std::move(item_));
    }

    //--------------------------------------------------------------------------
    void rule_stack(cref value) {
        static utf8string const field {"stack"};

        item_.stack = json::optional_int(value[field]).get_value_or(1);
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
//==============================================================================
class item_locale_parser : public json::common::locale {
public:
    //--------------------------------------------------------------------------
    explicit item_locale_parser(string_ref const filename)
      : locale {filename}
    {
        if (string_type != json::common::filetype_item) {
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
    operator item_def::localized_t::map_t&&() && {
        return std::move(strings_);
    }
private:
    item_def::locale             locale_;
    item_def::localized_t::map_t strings_;
};

} //namespace

item_material_def::definition_t
item_material_def::load_definitions(utf8string const& data) {
    return item_material_parser {data}.to_definition();
}

item_material_def::localized_t
item_material_def::load_localized_strings(utf8string const& data) {
    return item_material_locale_parser {data}.to_localized_string();
}


item_def::definition_t
item_def::load_definitions(string_ref const filename) {
    return item_def::definition_t {item_parser {filename}};
}

item_def::localized_t
item_def::load_localized_strings(string_ref const filename) {
    return item_def::localized_t {item_locale_parser {filename}};
}
