#pragma once

#include "types.hpp"
#include "font.hpp"

#include <array>
#include <vector>

#include <boost/format.hpp> //TODO ?
#include "messages.hpp"

#include "identifier.hpp"

namespace bkrl {

class font_face;
class renderer;
class message_map;

class item_definitions;
class item_stack;
class item_store;

namespace gui {

//==============================================================================
//==============================================================================
class item_list {
public:
    struct constants;

    item_list(
        font_face&              face
      , item_definitions const& item_defs
      , item_store       const& items
    );

    void reset(item_stack const& stack);

    void add_item(item_id id);

    void render(renderer& r, int x, int y);

    void clear();

    void select_next();

    void select_prev();

    int size() const noexcept { return static_cast<int>(items_.size()); }

    int selection() const noexcept { return selection_; }
    
    bool empty() const noexcept { return items_.empty(); }

    explicit operator bool() const noexcept { return !empty(); }
private:
    font_face*              face_       = nullptr;
    item_definitions const* item_defs_  = nullptr;
    item_store const*       item_store_ = nullptr;

    int  selection_ = 0;
    int  row_w_     = 0;
    int  row_h_     = 0;
    char prefix_    = 'a';

    utf8string name_buffer_;
    std::vector<transitory_text_layout> items_;
};

//==============================================================================
//==============================================================================
class message_log {
public:
    enum {line_count = 25};

    using format_t = boost::format;

    //--------------------------------------------------------------------------
    message_log(font_face& face, message_map const& msgs);

    //--------------------------------------------------------------------------
    void print_line(message_type msg);

    //--------------------------------------------------------------------------
    void print_line(string_ref str);

    //--------------------------------------------------------------------------
    template <typename Head, typename... Tail>
    void print_line(message_type const msg, Head&& head, Tail&&... tail) {
        auto format = format_t {get_message_string_(msg).data()};
        print_line_(format % head, std::forward<Tail>(tail)...);
    }

    void render(renderer& r, int x, int y);
private:
    string_ref get_message_string_(message_type msg) const;

    //--------------------------------------------------------------------------
    template <typename Head, typename... Tail>
    void print_line_(format_t& format, Head&& head, Tail&&... tail) {
        print_line_(format, std::forward<Head>(head), std::forward<Tail>(tail)...);
    }

    //--------------------------------------------------------------------------
    void print_line_(format_t& format);

    //--------------------------------------------------------------------------
    void make_line_(string_ref str);

    //--------------------------------------------------------------------------
    font_face*         font_face_ = nullptr;
    message_map const* msgs_      = nullptr;

    int front_ = 0;
    std::array<transitory_text_layout, line_count> lines_;
};

}} //namespace bkrl::gui
