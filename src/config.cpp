#include "config.hpp"
#include "json.hpp"
#include "random.hpp"

using namespace bkrl;

namespace {



class config_parser {
public:
    using cref = json::cref;

    explicit config_parser(cref data) {
        rule_root(data);
    }

    explicit config_parser(path_string_ref const filename)
      : config_parser {json::common::from_file(filename)}
    {
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
        json::common::get_filetype(value, json::common::filetype_config);
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
        static const utf8string field {"substantive_seed"};
        config_.substantive_seed = get_seed(value[field]);
    }

    //--------------------------------------------------------------------------
    void rule_trivial_seed(cref value) {
        static const utf8string field {"trivial_seed"};
        config_.substantive_seed = get_seed(value[field]);
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
        //TODO need to widen this
        //config_.font_name = json::require_string(value[field]).to_string();
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
