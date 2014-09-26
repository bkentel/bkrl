#include "config.hpp"
#include "json.hpp"

using namespace bkrl;

namespace {

class config_parser {
public:
    using cref = json::cref;

    explicit config_parser(string_ref filename) {
        auto const data = read_file(filename);
        
        std::string error;
        auto const root = json11::Json::parse(data, error);
        if (!error.empty()) {
            BK_TODO_FAIL();
        }

        rule_root(root);
    }

    void rule_root(cref value) {
        json::require_object(value);

        rule_filetype(value);
        rule_substantive_seed(value);
        rule_trivial_seed(value);
        rule_window_size(value);
        rule_window_pos(value);
        rule_font(value);
    }

    void rule_filetype(cref value) {
        json::common::get_filetype(value, "CONFIG");
    }

    void rule_substantive_seed(cref value) {
        static const utf8string field {"substantive_seed"};
        config_.substantive_seed = json::require_int<uint32_t>(value[field]);
    }

    void rule_trivial_seed(cref value) {
        static const utf8string field {"trivial_seed"};
        config_.trivial_seed = json::require_int<uint32_t>(value[field]);
    }

    void rule_window_size(cref value) {
        static const utf8string field {"window_size"};
        cref size = json::require_array(value[field], 2, 2);
        config_.window_w = json::require_int<uint32_t>(size[0]);
        config_.window_h = json::require_int<uint32_t>(size[1]);
    }

    void rule_window_pos(cref value) {
        static const utf8string field {"window_pos"};
        cref size = json::require_array(value[field], 2, 2);
        config_.window_x = json::require_int<int32_t>(size[0]);
        config_.window_y = json::require_int<int32_t>(size[1]);
    }

    void rule_font(cref value) {
        static const utf8string field {"font"};
        config_.font_name = json::require_string(value[field]).to_string();
    }

    operator config&&() {
        return std::move(config_);
    }
private:
    config config_;
};

}

config
bkrl::load_config(string_ref const filename) {
    return config_parser {filename};
}
