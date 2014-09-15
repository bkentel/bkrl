#include "keyboard.hpp"

#include "json11/json11.hpp"

using namespace bkrl;

using cref = json11::Json const&;

cref expect_object(cref json) {
    if (!json.is_object()) {
        BK_TODO_FAIL();
    }

    return json;
}

cref expect_array(cref json, size_t min_size = 0) {
    if (!json.is_array()) {
        BK_TODO_FAIL();
    }

    if (json.array_items().size() < min_size) {
        BK_TODO_FAIL();
    }

    return json;
}

template <typename T = int>
T expect_int(cref value) {
    using limits = std::numeric_limits<T>;

    if (!value.is_number()) {
        BK_TODO_FAIL();
    }

    double const value = value.number_value();
    
    static auto const min = static_cast<double>(limits::min());
    static auto const max = static_cast<double>(limits::max());

    if (value < min) {
        BK_TODO_FAIL();
    } else if (value > max) {
        BK_TODO_FAIL();
    }

    return static_cast<T>(value);
}

string_ref expect_string(cref value) {
    if (!value.is_string()) {
        BK_TODO_FAIL();
    }

    return value.string_value();
}

////////////////////////////////////////////////////////////////////////////////
// bkrl::keymap::impl_t implementation
////////////////////////////////////////////////////////////////////////////////
class keymap::impl_t {
public:
    using cref = json11::Json const&;

    explicit impl_t(string_ref filename);

    void rule_root(cref value);
    void rule_file_type(cref value);
    void rule_mappings(cref value);
    void rule_mapping_value(cref value);
private:
};

keymap::impl_t::impl_t(string_ref filename)
{
    auto const data = bkrl::read_file(filename);
    
    std::string error;
    auto const json = json11::Json::parse(data, error);

    if (!error.empty()) {
        BK_TODO_FAIL();
    }

    rule_root(json);
}

void keymap::impl_t::rule_root(cref value) {
    expect_object(value);
    rule_file_type(value);
    rule_mappings(value);
}

void keymap::impl_t::rule_file_type(cref value) {
    static std::string const key = {"file_type"};
    static std::string const expected_file_type = {"keymap"};

    auto const& file_type = expect_string(value[key]);

    if (file_type != expected_file_type) {
        BK_TODO_FAIL();
    }
}

void keymap::impl_t::rule_mappings(cref value) {
    static std::string const key = {"mappings"};
    
    auto const& mappings = expect_array(value[key]);

    for (auto const& mapping : mappings.array_items()) {
        rule_mapping_value(mapping);
    }
}

void keymap::impl_t::rule_mapping_value(cref value) {
    expect_array(value, 2);

    auto const command_name = expect_string(value[0]);
    auto const key_name     = expect_string(value[1]);

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
    return command_type::invalid;
}
