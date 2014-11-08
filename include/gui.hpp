#pragma once

#include "types.hpp"
#include "font.hpp"

#include <array>
#include <vector>

#include <boost/format.hpp> //TODO ?
#include "messages.hpp"

#include "identifier.hpp"
#include "render_types.hpp"

namespace bkrl {

class font_face;
class renderer;
class message_map;

class item_definitions;
class item_stack;
class item_store;

using item_list = std::vector<item_id>;

namespace gui {

namespace detail { class list_impl; }
namespace detail { class item_list_impl; }

//==============================================================================
//==============================================================================
class list {
public:
    explicit list(font_face& face);
    list(list&&);
    ~list();
    
    enum : int {
        row_header = -1
      , col_header = -1
      , auto_size = -1
    };

    void set_title(string_ref text);

    void set_col_header(int col, string_ref text);
    void set_row_header(int row, string_ref text);

    void set_position(ipoint2 p);

    void set_row_color(argb8 even, optional<argb8> odd);
    void set_selection_color(argb8 color);

    void add_row(string_ref header);
    void add_col(string_ref header);

    void set_col_width(int width = auto_size);
    void set_row_height(int height = auto_size);

    void set_text(int row, int col, string_ref text);
    void set_icon(int row, int col, int); //TODO
    
    void set_selection(int row);

    void select_next();
    void select_prev();

    void clear();

    void layout();

    void render(renderer& render);
private:
    std::unique_ptr<detail::list_impl> impl_;
};

class item_list {
public:
    item_list(item_list&&);
    ~item_list();

    item_list(
        font_face&              face
      , item_definitions const& item_defs
      , item_store       const& items
    );

    void set_position(ipoint2 p);
    void set_title(string_ref title);
    void render(renderer& r);
    int  index_at(int x, int y) const;

    item_id at(int index);
    
    void insert(item_id id);
    void insert(bkrl::item_list const& items);

    void clear();

    void set_selection(int i);
    void select_next();
    void select_prev();

    item_id selection() const noexcept;

    int  size()  const noexcept;
    bool empty() const noexcept;
private:
    std::unique_ptr<detail::item_list_impl> impl_;
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
    template <typename... Params>
    void print_line(message_type const msg, Params&&... params) {
        auto format = format_t {get_message_string_(msg).data()};
        print_line_(format, std::forward<Params>(params)...);
    }

    void render(renderer& r, int x, int y);
private:
    string_ref get_message_string_(message_type msg) const;

    //--------------------------------------------------------------------------
    template <typename Head, typename... Tail>
    void print_line_(format_t& format, Head&& head, Tail&&... tail) {
        format % head;
        print_line_(format, std::forward<Tail>(tail)...);
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
