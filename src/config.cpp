#include "config.hpp"
#include "json.hpp"
#include "random.hpp"

using namespace bkrl;

namespace {

class config_parser {
public:
    using cref = json::cref;

    explicit config_parser(string_ref const filename) {
        auto const root = json::common::from_file(filename);
        rule_root(root);
    }

    //--------------------------------------------------------------------------
    void rule_root(cref value) {
        json::require_object(value);

        rule_filetype(value);
        rule_substantive_seed(value);
        rule_trivial_seed(value);
        rule_window_size(value);
        rule_window_pos(value);
        rule_font(value);
    }

    //--------------------------------------------------------------------------
    void rule_filetype(cref value) {
        json::common::get_filetype(value, "CONFIG");
    }

    //--------------------------------------------------------------------------
    void rule_substantive_seed(cref value) {
        static const utf8string field {"substantive_seed"};

        config_.substantive_seed = json::optional_int<uint32_t>(value[field])
            .get_value_or(random::true_random());
    }

    //--------------------------------------------------------------------------
    void rule_trivial_seed(cref value) {
        static const utf8string field {"trivial_seed"};
        
        config_.trivial_seed = json::optional_int<uint32_t>(value[field])
            .get_value_or(random::true_random());
    }

    //--------------------------------------------------------------------------
    void rule_window_size(cref value) {
        static const utf8string field {"window_size"};

        if (!json::has_field(value, field)) {
            return;
        }

        cref size = json::require_array(value[field], 2, 2);

        config_.window_w = json::optional_int<uint32_t>(size[0]);
        config_.window_h = json::optional_int<uint32_t>(size[1]);
    }

    //--------------------------------------------------------------------------
    void rule_window_pos(cref value) {
        static const utf8string field {"window_pos"};
        
        if (!json::has_field(value, field)) {
            return;
        }

        cref size = json::require_array(value[field], 2, 2);

        config_.window_x = json::optional_int<int32_t>(size[0]);
        config_.window_y = json::optional_int<int32_t>(size[1]);
    }

    //--------------------------------------------------------------------------
    void rule_font(cref value) {
        static const utf8string field {"font"};
        config_.font_name = json::require_string(value[field]).to_string();
    }

    //--------------------------------------------------------------------------
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
