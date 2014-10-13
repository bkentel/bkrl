#pragma once

#include "types.hpp"
#include "util.hpp"
#include "locale.hpp"

namespace bkrl {

class item_material_def;
class item_color_def;
class item_type_def;
class item_tag_def;
class item_def;

template <typename T> class definition;

//==============================================================================
//==============================================================================
struct color_ref {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

//==============================================================================
//==============================================================================
class item_material_def : definition_base<item_material_def> {
public:
    struct locale {
        utf8string name;
        utf8string text;
    };

    static definition_t load_definitions(utf8string const& data);
    static localized_t  load_localized_strings(utf8string const& data);

    string_id id;
    string_id color;
    
    float     weight_mod;
    float     value_mod;
    
    std::vector<string_id> tags;
};

//==============================================================================
//==============================================================================
class item_color_def {
public:
    struct locale {
        utf8string name;
    };

    string_id id;
    color_ref color;
};

//==============================================================================
//==============================================================================
class item_def : public definition_base<item_def> {
public:
    struct locale {
        utf8string name;
        utf8string text;
    };

    static definition_t load_definitions(utf8string const& data);
    static definition_t load_definitions(string_ref filename);
    static localized_t  load_localized_strings(utf8string const& data);
    static localized_t  load_localized_strings(string_ref filename);

    string_id id;
    string_id type;
    string_id material;
    int       stack;
};

//==============================================================================
//==============================================================================
class item {
public:
    explicit item(identifier id)
      : id {id}
    {
    }

    bool operator<(item const& other) const {
        return id.hash < other.id.hash;
    }

    bool operator==(item const& other) const {
        return id.hash == other.id.hash;
    }

    bool operator!=(item const& other) const {
        return !(*this == other);
    }

    bool can_stack(item_def::definition_t const& defs) const {
        return defs[id].stack > 1;
    }

    string_ref name(item_def::localized_t const& locale) const {
        return locale(id, 0).name;
    }

    identifier id;
    int        count = 1;
};

} //namespace bkrl
