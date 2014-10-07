#include "keyboard.hpp"
#include "json.hpp"
#include "command_type.hpp"
#include "algorithm.hpp"
#include "iterable.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////
// bkrl::keymap::impl_t implementation
////////////////////////////////////////////////////////////////////////////////
class keymap::impl_t {
public:
    using cref = json::cref;

    explicit impl_t(string_ref filename);

    void rule_root(cref value);
    void rule_file_type(cref value);
    void rule_mappings(cref value);
    void rule_mapping_value(cref value);

    command_type operator[](key_combo const& key) const;
private:
    std::vector<key_mapping> mappings_;
};

keymap::impl_t::impl_t(string_ref const filename) {
    auto const json = json::common::from_file(filename);

    rule_root(json);

    bkrl::sort(mappings_, [](auto const& lhs, auto const& rhs) {
        return lhs.keys < rhs.keys;
    });
}

command_type
keymap::impl_t::operator[](key_combo const& key) const {
    auto const lower = bkrl::lower_bound(mappings_, key, [](auto& lhs, auto& rhs) {
        return lhs.keys.key < rhs.key; //match the key only; not mods
    });

    //for each matching key; not mods
    for (auto const& mapping : make_iterable(lower, std::end(mappings_))) {
        if (mapping.keys.key != key.key) {
            break;
        }

        if (mapping.keys.modifier.test(key.modifier)) {
            return mapping.command;
        }
    }

    return command_type::invalid;
}

void keymap::impl_t::rule_root(cref value) {
    json::require_object(value);

    rule_file_type(value);
    rule_mappings(value);
}

void keymap::impl_t::rule_file_type(cref value) {
    static utf8string const expected {"keymap"};
    json::common::get_filetype(value, expected);
}

void keymap::impl_t::rule_mappings(cref value) {
    static utf8string const field {"mappings"};
    
    cref mappings = json::require_array(value[field]);

    for (cref mapping : mappings.array_items()) {
        rule_mapping_value(mapping);
    }
}

void keymap::impl_t::rule_mapping_value(cref value) {
    json::require_array(value, 2);

    auto const command_name = json::require_string(value[0]);
    auto const key_name     = json::require_string(value[1]);

    auto const command_hash = slash_hash32(command_name);
    auto const key_hash     = slash_hash32(key_name);

    auto const command = enum_map<command_type>::get(command_hash);
    if (command.value == command_type::invalid) {
        BK_TODO_FAIL();
    }

    auto const key = enum_map<scancode>::get(key_hash);
    if (key.value == scancode::invalid) {
        BK_TODO_FAIL();
    }
    
    key_modifier mods {};

    for (auto i = 2u; i < value.array_items().size(); ++i) {
        auto const mod = json::require_string(value[i]);

        auto const kmod = enum_map<key_modifier_type>::get(mod).value;
        if (kmod == key_modifier_type::invalid) {
            BK_TODO_FAIL();
        }

        mods.set(kmod);
    }

    mappings_.emplace_back(
        key_mapping {
            key_combo {key.value, mods}
          , command.value
        }
    );
}

////////////////////////////////////////////////////////////////////////////////
// bkrl::keymap implementation
////////////////////////////////////////////////////////////////////////////////
keymap::keymap() {
}

keymap::keymap(string_ref filename)
  : impl_ {std::make_unique<impl_t>(filename)}
{
}

keymap::~keymap() = default;

command_type
keymap::operator[](key_combo const key) {
    return (*impl_)[key];
}
