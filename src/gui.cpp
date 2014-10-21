#include "gui.hpp"

#include "renderer.hpp"
#include "item.hpp"
#include "messages.hpp"

////////////////////////////////////////////////////////////////////////////////
// gui::item_list
////////////////////////////////////////////////////////////////////////////////

bkrl::gui::item_list::item_list(
    font_face&              face
  , item_definitions const& items
)
  : face_      {&face}
  , item_defs_ {&items}
{
}

void
bkrl::gui::item_list::reset(item_stack const& stack) {
    clear();

    for (auto&& item : stack) {
        add_item(item);
    }
}

void
bkrl::gui::item_list::add_item(
    item const& itm
) {
    auto const& def   = item_defs_->get_locale(itm.id);
    auto const& name  = def.name;

    name_buffer_.clear();
    name_buffer_.reserve(2 + name.size());

    name_buffer_.push_back(prefix_);
    name_buffer_.append({"\t-\t"});

    if (itm.count > 1) {
        name_buffer_.append(std::to_string(itm.count));
    }

    name_buffer_.push_back('\t');
    name_buffer_.append(name.data(), name.size());

    if (itm.damage_min) {
        BK_ASSERT_DBG(itm.damage_max);

        name_buffer_.append({" ["});
        name_buffer_.append(std::to_string(itm.damage_min));
        name_buffer_.append({" - "});
        name_buffer_.append(std::to_string(itm.damage_max));
        name_buffer_.append({"]"});
    }

    ////////

    items_.emplace_back(*face_, name_buffer_, 500, 32);

    auto const& back = items_.back();

    width_  =  std::max(width_, back.actual_width());
    height_ += back.actual_height();

    prefix_++;
}

void
bkrl::gui::item_list::render(
    renderer& r
  , int const x
  , int const y
) {
    if (empty()) {
        return;
    }

    constexpr auto border = 8;
        
    static auto color_background = make_color(50,  50,  50);
    static auto color_highlight  = make_color(150, 150, 50);

    auto cur_x = x + border;
    auto cur_y = y + border;

    auto const w = width_  + border * 2;
    auto const h = height_ + border * 2;

    auto const restore = r.restore_view();

    //draw the background
    r.set_draw_color(color_background);
    r.draw_filled_rect(make_rect_size(x, y, w, h));

    int i = 0;
    for (auto const& itm : items_) {
        if (i == selection_) {
            //draw the selection highlight
            r.set_draw_color(color_highlight);
            r.draw_filled_rect(make_rect_size(cur_x, cur_y, w - border * 2, itm.actual_height()));
            r.set_draw_color(color_background);
        }

        itm.render(r, *face_, cur_x, cur_y);
        cur_y += itm.actual_height();
        ++i;
    }
}

void
bkrl::gui::item_list::clear() {
    items_.clear();
    selection_ = 0;
    width_     = 0;
    height_    = 0;
    prefix_   = 'a';
}

void
bkrl::gui::item_list::select_next() {
    auto const size = items_.size();
    selection_ = (selection_ + 1) % size;
}

void
bkrl::gui::item_list::select_prev() {
    auto const size = items_.size();
    selection_ = (selection_ == 0)
      ? (size - 1)
      : (selection_ - 1);
}

////////////////////////////////////////////////////////////////////////////////
// gui::message_log
////////////////////////////////////////////////////////////////////////////////

bkrl::gui::message_log::message_log(font_face& face, message_map const& msgs)
  : font_face_ {&face}
  , msgs_      {&msgs}
{
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::print_line(message_type const msg) {
    make_line_(get_message_string_(msg));
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::print_line(string_ref const str) {
    make_line_(str);
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::render(
    renderer& r
  , int x
  , int y
) {
    constexpr auto max_line = 3;

    auto const restore = r.restore_view();

    auto beg = front_ - max_line;
    beg = (beg < 0) ? (line_count + beg) : (beg);

    auto const end = front_;

    for (auto i = beg; i != end; i = (i + 1) % line_count) {
        auto const& line = lines_[i];
        if (line.empty()) {
            continue;
        }

        line.render(r, *font_face_, x, y);
        y += line.actual_height();
    }
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::print_line_(format_t& format) {
    make_line_(boost::str(format));
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::make_line_(string_ref const str) {
    constexpr auto max_w = 1000;
    auto const max_h = font_face_->line_gap();

    auto& line = lines_[front_++];
    if (front_ >= line_count) {
        front_ = 0;
    }

    line.reset(*font_face_, str, max_w, max_h);
}

bkrl::string_ref
bkrl::gui::message_log::get_message_string_(message_type msg) const {
    return (*msgs_)[msg];
}
