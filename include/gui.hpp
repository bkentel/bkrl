#pragma once

#include <array>
#include <vector>

#include <boost/format.hpp>

#include "font.hpp"
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
//!
//==============================================================================
class list {
public:
    explicit list(font_face& face);
    list(list&&);
    ~list();
    
    ipoint2 get_index_at(ipoint2 p) const;

    int get_selection() const;

    void set_title(string_ref text);

    void set_col_header(int col, string_ref text);
    void set_row_header(int row, string_ref text);

    void set_position(ipoint2 p);

    //void set_row_color(argb8 even, optional<argb8> odd);
    //void set_selection_color(argb8 color);

    int add_row(string_ref header);
    int add_col(string_ref header);

    //void set_col_width(int width = auto_size);
    //void set_row_height(int height = auto_size);

    void set_text(int row, int col, string_ref text);
    //void set_icon(int row, int col, int); //TODO
    
    void set_selection(int row);
    
    void select_next();
    void select_prev();

    void clear();

    void layout();

    void render(renderer& render);
private:
    std::unique_ptr<detail::list_impl> impl_;
};

//==============================================================================
//!
//==============================================================================
class item_list {
public:
    item_list(item_list&&);
    ~item_list();

    item_list(
        font_face&              face
      , item_definitions const& item_defs
      , item_store       const& items
      , message_map      const& messages
    );

    ///////

    int get_index_at(ipoint2 p) const;

    int get_selection() const;

    void set_title(string_ref text);

    void set_position(ipoint2 p);
   
    void set_selection(int row);
    
    void select_next();
    void select_prev();

    void clear();

    void layout();

    void render(renderer& render);

    ///////

    item_id at(int index);
    
    void insert(item_id id);
    virtual void insert(bkrl::item_list const& items);

    int  size()  const noexcept;
    bool empty() const noexcept;
protected:
    std::unique_ptr<detail::item_list_impl> impl_;
};

//==============================================================================
//!
//==============================================================================
class equip_list : public item_list {
public:
    equip_list(equip_list&&) = default;
    ~equip_list() = default;

    using item_list::item_list;
   
    void insert(bkrl::item_list const& items) override;
};

//==============================================================================
//!
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

//==============================================================================
//!
//==============================================================================
class map_inspect {
public:
    map_inspect()                              = default;
    map_inspect(map_inspect const&)            = delete;
    map_inspect(map_inspect&&)                 = default;
    map_inspect& operator=(map_inspect const&) = delete;
    map_inspect& operator=(map_inspect&&)      = default;

    void show() noexcept { visible_ = true; }

    void show(int const x, int const y) noexcept {
        show();

        render_x_ = x;
        render_y_ = y;
    }

    void hide() noexcept { visible_ = false; }

    void reset(font_face& face, string_ref msg);

    void set_view_size(int const w, int const h) {
        BK_ASSERT(w > 0 && h > 0);

        view_w_ = w;
        view_h_ = h;
    }

    void render(renderer& r);
private:
    transitory_text_layout text_;

    font_face* font_face_ = nullptr;
    int        view_w_  = 0;
    int        view_h_  = 0;
    int        render_x_  = 0;
    int        render_y_  = 0;
    int        render_w_  = 0;
    int        render_h_  = 0;
    bool       visible_   = false;
};

}} //namespace bkrl::gui
