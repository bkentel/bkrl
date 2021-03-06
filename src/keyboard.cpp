#include "keyboard.hpp"
#include "json.hpp"
#include "command_type.hpp"
#include "algorithm.hpp"
#include "iterable.hpp"
#include "hash.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////
// bkrl::keymap::impl_t implementation
////////////////////////////////////////////////////////////////////////////////
class keymap::impl_t {
public:
    using cref = json::cref;

    explicit impl_t(json::cref data);

    void rule_root(cref value);
    void rule_file_type(cref value);
    void rule_mappings(cref value);
    void rule_mapping_value(cref value);

    command_type operator[](key_combo const& key) const;
private:
    std::vector<key_mapping> mappings_;
};

keymap::impl_t::impl_t(json::cref data) {
    rule_root(data);

    bkrl::sort(mappings_, [](auto const& lhs, auto const& rhs) {
        return lhs.keys < rhs.keys;
    });
}

command_type
keymap::impl_t::operator[](key_combo const& key) const {
    auto const result = bkrl::lower_bound(mappings_, key, [](auto& lhs, auto& rhs) {
        return lhs.keys.key < rhs.key; //match the key only; not mods
    });

    if (!result.second) {
        return command_type::invalid;
    }

    auto const& it = result.first;

    //TODO
    //for each matching key; not mods
    for (auto const& mapping : make_iterable(it, std::end(mappings_))) {
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
    json::common::get_filetype(value, json::common::filetype_keymap);
}

void keymap::impl_t::rule_mappings(cref value) {
    auto const& field = json::common::field_mappings;
    
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

    auto const command = from_hash<command_type>(command_hash);
    if (command == command_type::invalid) {
        BK_TODO_FAIL();
    }

    auto const key = from_hash<scancode>(key_hash);
    if (key == scancode::invalid) {
        BK_TODO_FAIL();
    }
    
    key_modifier mods {};

    for (auto i = 2u; i < value.array_items().size(); ++i) {
        auto const mod = json::require_string(value[i]);
        auto const mod_hash = slash_hash32(mod);

        auto const kmod = from_hash<key_modifier_type>(mod_hash);
        if (kmod == key_modifier_type::invalid) {
            BK_TODO_FAIL();
        }

        mods.set(kmod);
    }

    mappings_.emplace_back(
        key_mapping {key_combo {key, mods}, command}
    );
}

////////////////////////////////////////////////////////////////////////////////
// bkrl::keymap implementation
////////////////////////////////////////////////////////////////////////////////
keymap::keymap() = default;
keymap::keymap(keymap&&) = default;
keymap& keymap::operator=(keymap&&) = default;
keymap::~keymap() = default;

keymap::keymap(json::cref data)
  : impl_ {std::make_unique<impl_t>(data)}
{
}

command_type
keymap::operator[](key_combo const key) const {
    return (*impl_)[key];
}
