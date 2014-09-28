#include "keyboard.hpp"
#include "json.hpp"
#include "command_type.hpp"

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

keymap::impl_t::impl_t(string_ref filename) {
    auto const data = bkrl::read_file(filename);
    
    std::string error;
    auto const json = json11::Json::parse(data, error);

    if (!error.empty()) {
        BK_TODO_FAIL();
    }

    rule_root(json);

    std::sort(
        std::begin(mappings_)
      , std::end(mappings_)
      , [](key_mapping const& lhs, key_mapping const& rhs) {
            return lhs.keys < rhs.keys;
        }
    );
}

command_type
keymap::impl_t::operator[](key_combo const& key) const {
    auto const lower = std::lower_bound(
        std::cbegin(mappings_)
      , std::cend(mappings_)
      , key
      , [](key_mapping const& lhs, key_combo const& rhs) {
            //match the key only; not mods
            return lhs.keys.key < rhs.key;
        }
    );

    //for each matching key; not mods
    for (auto it = lower; it != std::cend(mappings_); ++it) {
        if (it->keys.key != key.key) {
            break;
        }

        if (it->keys.modifier.test(key.modifier)) {
            return it->command;
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
    
    key_modifier mods;

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
