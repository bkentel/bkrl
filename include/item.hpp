#pragma once

#include "types.hpp"
#include "util.hpp"
#include <boost/container/flat_map.hpp>

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
template <typename T>
class localized_string {
public:
    BK_NOCOPY(localized_string);
    BK_DEFMOVE(localized_string);

    using locale_t = typename T::locale;
    //using map_t = boost::container::flat_map<
    //    hash_t
    //  , boost::container::flat_map<hash_t, locale_t>
    //>;

    using map_t = boost::container::flat_map<hash_t, locale_t>;

    localized_string() = default;

    explicit localized_string(map_t&& data)
      : data_ {std::move(data)}
    {
    }

    //explicit localized_string(string_ref const filename)
    //  : localized_string(bkrl::read_file(filename))
    //{
    //}
    //
    //explicit localized_string(utf8string const& data)
    //  : data_ {std::move(T::load_localized_strings(data).data_)}
    //{
    //}

    locale_t const& operator()(hash_t const key, hash_t const lang) const {
        auto const it = data_.find(key);
        if (it == std::cend(data_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
private:
    map_t data_;
};

//==============================================================================
//==============================================================================
template <typename T>
class definition {
public:
    BK_NOCOPY(definition);
    BK_DEFMOVE(definition);

    using map_t = boost::container::flat_map<hash_t, T>;

    definition() = default;

    explicit definition(map_t&& data)
      : data_ {std::move(data)}
    {
    }

    //explicit definition(string_ref const filename)
    //  : definition(bkrl::read_file(filename))
    //{
    //}
    //
    //explicit definition(utf8string const& data)
    //  : data_ {std::move(T::load_definitions(data).data_)}
    //{
    //}

    T const& operator[](hash_t const key) const {
        auto const it = data_.find(key);

        if (it == std::cend(data_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }

    size_t size() const noexcept {
        return data_.size();
    }

    //TODO temp for testing
    T const& at_index(size_t index) const {
        auto it = data_.cbegin();
        std::advance(it, index);
        return it->second;
    }
private:
    map_t data_;
};

//==============================================================================
//==============================================================================
template <typename T>
struct definition_base {
    using definition_t = definition<T>;
    using localized_t  = localized_string<T>;

    BK_NOCOPY(definition_base);
    BK_DEFMOVE(definition_base);

    definition_base() = default;
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
};

//==============================================================================
//==============================================================================
class item {
public:
    string_id id;
};

} //namespace bkrl
