#include "messages.hpp"
#include "json.hpp"

#include <vector>

using namespace bkrl;

namespace {
class message_parser {
public:
    using cref = json::cref;

    using key_t   = message_type;
    using value_t = utf8string;
    using container_t = boost::container::flat_map<key_t, value_t>;

    explicit message_parser(string_ref const filename) {
        auto const value = json::common::from_file(filename);
        rule_root(value);
    }

    void rule_root(cref value) {
        json::require_object(value);

        rule_file_type(value);
        rule_string_type(value);
        rule_language(value);
        rule_definitions(value);
    }
    
    void rule_file_type(cref value) {
        json::common::get_filetype(value, "LOCALE");
    }
    
    void rule_string_type(cref value) {
        static utf8string const field {"string_type"};
        static utf8string const expected {"MESSAGE"};
        
        auto const type = json::require_string(value[field]);

        if (type != expected) {
            BK_TODO_FAIL();
        }
    }
    
    void rule_language(cref value) {
        static utf8string const field {"language"};
        
        lang_ = json::require_string(value[field]).to_string();
    }
    
    void rule_definitions(cref value) {
        static utf8string const field {"definitions"};
        auto const defs = json::require_array(value[field]);

        messages_.reserve(enum_value(message_type::enum_size));

        for (auto const& def : defs.array_items()) {
            rule_definition(def);
        }
    }
    
    void rule_definition(cref value) {
        auto const arr = json::require_array(value, 2, 2);
        
        auto const id  = json::require_string(arr[0]);
        auto const str = json::require_string(arr[1]);

        auto const hash   = slash_hash32(id);
        auto const mapped = enum_map<message_type>::get(hash);

        if (mapped.value == message_type::invalid) {
            BK_TODO_FAIL();
        } else if (mapped.string != id) {
            BK_TODO_FAIL();
        }

        messages_.emplace(mapped.value, str.to_string());
    }

    string_ref language() const {
        return lang_;
    }

    container_t& messages() {
        return messages_;
    }


private:
    utf8string lang_;
    container_t messages_;
};

}

class message_map::impl_t {
public:
    impl_t(string_ref const filename)
    {
        message_parser parser {filename};

        messages_ = std::move(parser.messages());
    }

    string_ref operator()(message_type const msg, hash_t const lang) const {
        static string_ref const undefined {"<undefined message>"};

        auto const it = messages_.find(msg);
        if (it == std::cend(messages_)) {
            return undefined;
        }

        return it->second;
    }
private:
    message_parser::container_t messages_;
};

message_map::message_map(string_ref const filename)
  : impl_ {std::make_unique<impl_t>(filename)}
{
}

message_map::message_map(utf8string const& data)
{
}

message_map::~message_map() = default;

string_ref message_map::operator()(message_type const msg, hash_t const lang) const {
    return (*impl_)(msg, lang);
}
