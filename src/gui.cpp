#include "gui.hpp"

#include "renderer.hpp"
#include "items.hpp"
#include "messages.hpp"

////////////////////////////////////////////////////////////////////////////////
// gui::item_list
////////////////////////////////////////////////////////////////////////////////
class bkrl::gui::detail::item_list_impl {
public:
    static constexpr int border  = 8;
    static constexpr int padding = 4;

    item_list_impl(
        font_face&              face
      , item_definitions const& item_defs
      , item_store       const& items
    )
      : face_       {&face}
      , item_defs_  {&item_defs}
      , item_store_ {&items}
    {
    }

    void set_title(string_ref const title) {
        title_.reset(*face_, title);
        row_w_ = std::max(row_w_, title_.actual_width()  + padding);
        row_h_ = std::max(row_h_, title_.actual_height() + padding);
    }

    void render(renderer& r, int const x, int const y) {
        auto const size = static_cast<int>(items_.size());

        if (size == 0) {
            return;
        }
        
        static auto color_background = make_color(200,  200,  200);
        static auto color_even       = make_color(60,  60,  60);
        static auto color_odd        = make_color(80,  80,  80);
        static auto color_highlight  = make_color(150, 150, 120);

        auto cur_x = x + border;
        auto cur_y = y + border;

        auto const w = row_w_ + border * 2;
        auto const h = row_h_ * (size + 1) + border * 2;

        auto const restore = r.restore_view();

        //draw the background
        r.set_draw_color(color_background);
        r.draw_filled_rect(make_rect_size(x, y, w, h));

        title_.render(r, *face_, cur_x, cur_y);
        cur_y += row_h_;

        int i = 0;
        for (auto const& itm : item_text_) {
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
    
    int index_at(int x, int y) const {
        return 0;
    }

    item_id at(int const index) {
        BK_ASSERT(index >= 0 && index < items_.size());
        return items_[index];
    }

    void insert(item_id const id) {
        auto const& itm  = (*item_store_)[id];
        auto const& loc  = item_defs_->get_locale(itm.id);
        auto const& name = loc.name;

        name_buffer_.clear();
        name_buffer_.reserve(2 + name.size());

        name_buffer_.push_back(prefix_);
        name_buffer_.append({"\t-\t"});

        name_buffer_.push_back('\t');
        name_buffer_.append(name.data(), name.size());

        items_.emplace_back(id);
        item_text_.emplace_back(*face_, name_buffer_, 500, 32);
        auto const& back = item_text_.back();

        row_w_ = std::max(row_w_, back.actual_width()  + padding);
        row_h_ = std::max(row_h_, back.actual_height() + padding);

        prefix_++;
    }

    void insert(item_stack const& stack) {
        clear();

        for (auto&& iid : stack) {
            insert(iid);
        }
    }

    void clear() {
        items_.clear();
        item_text_.clear();
        title_.clear();

        selection_ = 0;
        row_w_     = 0;
        row_h_     = 0;
        prefix_   = 'a';
    }

    void select_next() {
        auto const size = items_.size();
        selection_ = (selection_ + 1) % size;
    }

    void select_prev() {
        auto const size = items_.size();
        selection_ = (selection_ == 0)
          ? (size - 1)
          : (selection_ - 1);
    }

    item_id selection() const noexcept {
        return items_[selection_];
    }

    int size() const noexcept {
        return items_.size();
    }
    
    bool empty() const noexcept {
        return items_.empty();
    }
private:
    font_face*              face_       = nullptr;
    item_definitions const* item_defs_  = nullptr;
    item_store const*       item_store_ = nullptr;

    int  selection_ = 0;
    int  row_w_     = 0;
    int  row_h_     = 0;
    char prefix_    = 'a';

    utf8string name_buffer_;

    transitory_text_layout              title_;
    
    std::vector<item_id>                items_;
    std::vector<transitory_text_layout> item_text_;
};

bkrl::gui::item_list::item_list(item_list&&) = default;
bkrl::gui::item_list::~item_list() = default;

bkrl::gui::item_list::item_list(
    font_face&              face
  , item_definitions const& item_defs
  , item_store       const& items
)
  : impl_ {std::make_unique<detail::item_list_impl>(face, item_defs, items)}
{
}

void bkrl::gui::item_list::set_title(string_ref const title) {
    impl_->set_title(title);
}

void bkrl::gui::item_list::render(renderer& r, int const x, int const y) {
    impl_->render(r, x, y);
}

int bkrl::gui::item_list::index_at(int const x, int const y) const {
    return impl_->index_at(x, y);
}

bkrl::item_id bkrl::gui::item_list::at(int const index) {
    return impl_->at(index);
}

void bkrl::gui::item_list::insert(item_stack const& stack) {
    impl_->insert(stack);
}

void bkrl::gui::item_list::insert(item_id const id) {
    impl_->insert(id);
}

void bkrl::gui::item_list::clear() {
    impl_->clear();
}

void bkrl::gui::item_list::select_next() {
    impl_->select_next();
}

void bkrl::gui::item_list::select_prev() {
    impl_->select_prev();
}

bkrl::item_id bkrl::gui::item_list::selection() const noexcept {
    return impl_->selection();
}

int bkrl::gui::item_list::size() const noexcept {
    return impl_->size();
}

bool bkrl::gui::item_list::empty() const noexcept {
    return impl_->empty();
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
