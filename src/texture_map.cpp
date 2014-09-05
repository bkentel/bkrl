#include "texture_type.hpp"
#include "util.hpp"
#include "assert.hpp"

#include "json11/json11.hpp"

////////////////////////////////////////////////////////////////////////////////
// bkrl::texture_map
////////////////////////////////////////////////////////////////////////////////
namespace {
    using texture_map = ::bkrl::texture_map;
}

struct texture_map::impl_t {
    using json_value = json11::Json;

    using value_type = std::pair<texture_type, texture_id>;
    std::vector<value_type> mappings_;

    explicit impl_t(std::string const& source) {
        std::string err;
        auto root = json11::Json::parse(source, err);

        rule_root(root);

        //sort by type
        bkrl::sort(mappings_, [](value_type const& a, value_type const& b) {
            return a.first < b.first;
        });

        //check for duplicates
        auto const it = std::adjacent_find(
            std::cbegin(mappings_), std::cend(mappings_)
        );

        if (it != std::cend(mappings_)) {
            BK_TODO_FAIL(); //duplicate mapping
        }

        //check for missing values
        if (mappings_.size() != enum_value(texture_type::enum_size)) {
            for (size_t i = 0; i < mappings_.size(); ++i) {
                if (i != enum_value(mappings_[i].first)) {
                    BK_TODO_FAIL(); //fill in defaulted missing values
                }
            }
        }
    }

    texture_id operator[](texture_type const t) const {
        return mappings_[static_cast<size_t>(t)].second;
    }

    void rule_root(json_value const& value) {
        if (!value.is_object()) {
            BK_TODO_FAIL();
        }

        rule_file_type(value);
        rule_mappings(value);
    }

    void rule_file_type(json_value const& value) {
        static std::string const key_file_type {"file_type"};

        auto const file_type = value[key_file_type];
        if (!file_type.is_string()) {
            BK_TODO_FAIL();
        }
    }

    void rule_mappings(json_value const& value) {
        static std::string const key_mappings {"mappings"};

        auto const mappings = value[key_mappings];
        if (!mappings.is_array()) {
            BK_TODO_FAIL();
        }

        for (auto const& mapping_pair : mappings.array_items()) {
            rule_mapping_pair(mapping_pair);
        }
    }

    void rule_mapping_pair(json_value const& value) {
        if (!value.is_array()) {
            BK_TODO_FAIL();
        } else if (value.array_items().size() != 2) {
            BK_TODO_FAIL();
        }

        auto const& array = value.array_items();
        if (!array[0].is_string()) {
            BK_TODO_FAIL();
        }
        if (!array[1].is_number()) {
            BK_TODO_FAIL();
        }

        auto const& str  = array[0].string_value();
        auto const  hash = slash_hash32(str);
        auto const  e    = enum_map<texture_type>::get(hash);

        if (e.value == texture_type::invalid) {
            if (hash != e.hash) {
                BK_TODO_FAIL();
            }
        }

        mappings_.emplace_back(
            e.value, array[1].int_value()
        );
    }
};

texture_map::~texture_map() = default;

texture_map::texture_map(std::string const& source)
  : impl_ {std::make_unique<impl_t>(source)}
{
}

bkrl::texture_id texture_map::operator[](bkrl::texture_type type) const {
    return (*impl_)[type];
}
