#include "gui.hpp"

#include "renderer.hpp"
#include "items.hpp"
#include "messages.hpp"

////////////////////////////////////////////////////////////////////////////////
// gui::item_list
////////////////////////////////////////////////////////////////////////////////
struct bkrl::gui::item_list::constants {
    static constexpr int border  = 8;
    static constexpr int padding = 4;
};

bkrl::gui::item_list::item_list(
    font_face&              face
  , item_definitions const& item_defs
  , item_store       const& items
)
  : face_       {&face}
  , item_defs_  {&item_defs}
  , item_store_ {&items}
{
}

void bkrl::gui::item_list::set_title(string_ref const title) {
    title_.reset(*face_, title);
    row_w_ = std::max(row_w_, title_.actual_width()  + constants::padding);
    row_h_ = std::max(row_h_, title_.actual_height() + constants::padding);
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
    item_id const id
) {
    auto const& itm  = (*item_store_)[id];
    auto const& loc  = item_defs_->get_locale(itm.id);
    auto const& name = loc.name;

    name_buffer_.clear();
    name_buffer_.reserve(2 + name.size());

    name_buffer_.push_back(prefix_);
    name_buffer_.append({"\t-\t"});

    name_buffer_.push_back('\t');
    name_buffer_.append(name.data(), name.size());

    items_.emplace_back(*face_, name_buffer_, 500, 32);

    auto const& back = items_.back();

    row_w_ = std::max(row_w_, back.actual_width()  + constants::padding);
    row_h_ = std::max(row_h_, back.actual_height() + constants::padding);

    prefix_++;
}

void
bkrl::gui::item_list::render(
    renderer& r
  , int const x
  , int const y
) {
    auto const size = static_cast<int>(items_.size());

    if (size == 0) {
        return;
    }
        
    static auto color_background = make_color(200,  200,  200);
    static auto color_even       = make_color(60,  60,  60);
    static auto color_odd        = make_color(80,  80,  80);
    static auto color_highlight  = make_color(150, 150, 120);

    auto cur_x = x + constants::border;
    auto cur_y = y + constants::border;

    auto const w = row_w_ + constants::border * 2;
    auto const h = row_h_ * (size + 1) + constants::border * 2;

    auto const restore = r.restore_view();

    //draw the background
    r.set_draw_color(color_background);
    r.draw_filled_rect(make_rect_size(x, y, w, h));

    title_.render(r, *face_, cur_x, cur_y);
    cur_y += row_h_;

    int i = 0;
    for (auto const& itm : items_) {
        if (i == selection_) {
            r.set_draw_color(color_highlight);
        } else if (i % 2 == 0) {
            r.set_draw_color(color_even);
        } else {
            r.set_draw_color(color_odd);
        }

        r.draw_filled_rect(make_rect_size(cur_x, cur_y, row_w_, row_h_));

        itm.render(r, *face_, cur_x, cur_y);
        cur_y += row_h_;
        ++i;
    }
}

void
bkrl::gui::item_list::clear() {
    items_.clear();
    title_.clear();

    selection_ = 0;
    row_w_     = 0;
    row_h_     = 0;
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
