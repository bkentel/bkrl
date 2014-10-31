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

namespace detail { class item_list_impl; }

//==============================================================================
//==============================================================================
class item_list {
public:
    item_list(item_list&&);
    ~item_list();

    item_list(
        font_face&              face
      , item_definitions const& item_defs
      , item_store       const& items
    );

    void set_title(string_ref title);
    void render(renderer& r, int x, int y);
    int  index_at(int x, int y) const;

    item_id at(int index);
    
    void insert(item_id id);
    void insert(item_stack const& stack);

    void clear();

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
