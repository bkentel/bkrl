#include "config.hpp"
#include "json.hpp"
#include "random.hpp"

namespace jc = bkrl::json::common;
using namespace bkrl;

namespace {

class config_parser {
public:
    using cref = json::cref;

    explicit config_parser(cref data) {
        rule_root(data);
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
        rule_language(value);
    }

    //--------------------------------------------------------------------------
    void rule_filetype(cref value) {
        json::common::get_filetype(value, jc::filetype_config);
    }

    static hash_t get_seed(cref value) {
        switch (value.type()) {
        case json11::Json::Type::NUL :
            return random::true_random();
        case json11::Json::Type::STRING :
            return slash_hash32(value.string_value());
        case json11::Json::Type::NUMBER :
            return json::require_int<hash_t>(value);
        default:
            BK_DEBUG_BREAK();
            break;
        }

        return 0;
    }

    //--------------------------------------------------------------------------
    void rule_substantive_seed(cref value) {
        config_.substantive_seed = get_seed(value[jc::field_substantive_seed]);
    }

    //--------------------------------------------------------------------------
    void rule_trivial_seed(cref value) {
        config_.substantive_seed = get_seed(value[jc::field_trivial_seed]);
    }

    //--------------------------------------------------------------------------
    void rule_window_size(cref value) {
        if (!json::has_field(value, jc::field_window_size)) {
            return;
        }

        cref size = json::require_array(value[jc::field_window_size], 2, 2);

        config_.window_w = json::optional_int<uint32_t>(size[0]);
        config_.window_h = json::optional_int<uint32_t>(size[1]);
    }

    //--------------------------------------------------------------------------
    void rule_window_pos(cref value) {       
        if (!json::has_field(value, jc::field_window_pos)) {
            return;
        }

        cref size = json::require_array(value[jc::field_window_pos], 2, 2);

        config_.window_x = json::optional_int<int32_t>(size[0]);
        config_.window_y = json::optional_int<int32_t>(size[1]);
    }

    //--------------------------------------------------------------------------
    void rule_font(cref value) {
        config_.font_name = json::common::get_path_string(value[jc::field_font]);
    }

    //--------------------------------------------------------------------------
    void rule_language(cref value) {
        auto const lang = json::common::get_locale(value);
        config_.language  = lang ? *lang : BK_MAKE_LANG_CODE2('e','n');
    }

    //--------------------------------------------------------------------------
    operator config&&() && {
        return std::move(config_);
    }
private:
    config config_;
};

}

config bkrl::load_config(json::cref data) {
    return config_parser {data};
}
