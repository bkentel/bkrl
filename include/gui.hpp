#pragma once

#include "types.hpp"
#include "font.hpp"

#include <array>
#include <vector>

namespace bkrl {

class font_face;
class item_definitions;
class item;
class item_stack;
class renderer;
class message_map;

namespace gui {

//==============================================================================
//==============================================================================
class item_list {
public:
    item_list(font_face& face, item_definitions const& items);

    void reset(item_stack const& stack);

    void add_item(item const& itm);

    void render(renderer& r, int x, int y);

    void clear();

    void select_next();

    void select_prev();

    int size() const noexcept { return static_cast<int>(items_.size()); }

    int selection() const noexcept { return selection_; }
    
    bool empty() const noexcept { return items_.empty(); }

    explicit operator bool() const noexcept { return !empty(); }
private:
    font_face*              face_      = nullptr;
    item_definitions const* item_defs_ = nullptr;

    int  selection_ = 0;
    int  width_     = 0;
    int  height_    = 0;
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
        auto const& str = (*msgs_)[msg];

        format_t format {str.data()};

        print_line_(format % head, std::forward<Tail>(tail)...);
    }

    void render(renderer& r, int x, int y);
private:
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
