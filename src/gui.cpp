#include "gui.hpp"

#include "renderer.hpp"
#include "items.hpp"
#include "messages.hpp"

#include <boost/container/flat_map.hpp>

////////////////////////////////////////////////////////////////////////////////
// gui::detail::list_impl
////////////////////////////////////////////////////////////////////////////////
class bkrl::gui::detail::list_impl {
public:
    struct key_t {
        int row;
        int col;

        friend inline bool operator<(key_t const& lhs, key_t const& rhs) noexcept {
            return std::make_tuple(lhs.row, lhs.col)
                 < std::make_tuple(rhs.row, rhs.col);            
        }
    };

    explicit list_impl(font_face& face)
      : face_ {&face}
    {
        update_entry_(0, 0, "\xE2\x8C\x98");
    }
    
    ipoint2 get_index_at(ipoint2 p) const;

    int get_selection() const;

    void set_title(string_ref text);

    void set_col_header(int col, string_ref text);
    void set_row_header(int row, string_ref text);

    void set_position(ipoint2 p);

    void set_row_color(argb8 even, optional<argb8> odd);
    void set_selection_color(argb8 color);

    void add_row(string_ref header);
    void add_col(string_ref header);

    void set_col_width(int width);
    void set_row_height(int height);

    void set_text(int row, int col, string_ref text);
    void set_icon(int row, int col, int); //TODO
    
    void set_selection(int row);

    void select_next();
    void select_prev();

    void clear();

    void layout();

    void render(renderer& render);
private:
    struct cell_info {
        int size;
        int offset;
    };

    void update_entry_(int row, int col, string_ref text);

    font_face* face_      = nullptr;
    int        selection_ = 0;
    ipoint2    pos_       = ipoint2 {0, 0};
    int        rows_      = 0;
    int        cols_      = 0;
    int        total_w_   = 0;
    int        total_h_   = 0;

    std::vector<cell_info> row_info_;
    std::vector<cell_info> col_info_;

    transitory_text_layout title_;
    
    boost::container::flat_map<key_t, transitory_text_layout, std::less<>> entries_;

    int border_size_        = 4;
    int title_border_size_  = 4;
    int padding_col_        = 8;
    int padding_row_        = 0;

    argb8 color_even_ = argb8 {255, 30, 30, 30};
    argb8 color_odd_  = argb8 {255, 40, 40, 40};
    argb8 color_sel_  = argb8 {255, 100, 100, 20};
    argb8 color_back_ = argb8 {255, 80, 60, 60};
};

bkrl::ipoint2 bkrl::gui::detail::list_impl::get_index_at(ipoint2 const p) const {
    BK_TODO_FAIL();
    return p;
}

int bkrl::gui::detail::list_impl::get_selection() const {
    return selection_;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::update_entry_(int const row, int const col, string_ref const text) {
    if (row < 0 || row > rows_) {
        BK_TODO_FAIL();
    }

    if (col < 0 || col > cols_) {
        BK_TODO_FAIL();
    }

    auto const& entry = [&]() -> transitory_text_layout const& {
        auto const key = key_t {row, col};
        auto const it = entries_.find(key);

        if (it == std::end(entries_)) {
            auto const result = entries_.emplace(
                key
              , transitory_text_layout {*face_, text}
            );

            if (!result.second) {
                BK_TODO_FAIL();
            }

            auto const ri = static_cast<size_t>(row);
            auto const ci = static_cast<size_t>(col);

            if (ri >= row_info_.size()) {
                row_info_.resize(ri + 1, cell_info {0, 0});
            }

            if (ci >= col_info_.size()) {
                col_info_.resize(ci + 1, cell_info {0, 0});
            }

            return result.first->second;
        } else {
            it->second.reset(*face_, text);
            return it->second;
        }
    }();

    auto& row_size = row_info_[row].size;
    auto& col_size = col_info_[col].size;

    row_size = face_->line_gap();
    //row_size = std::max(row_size, entry.actual_height());
    col_size = std::max(col_size, entry.actual_width());
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_title(string_ref const text) {
    title_.reset(*face_, text);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_col_header(int const col, string_ref const text) {
    update_entry_(0, col + 1, text);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_row_header(int const row, string_ref const text) {
    update_entry_(row + 1, 0, text);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_position(ipoint2 const p) {
    pos_ = p;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_row_color(argb8 const even, optional<argb8> const odd) {
    color_even_ = even;
    color_odd_  = odd ? *odd : even;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_selection_color(argb8 const color) {
    color_sel_ = color;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::add_row(string_ref const header) {
    set_row_header(rows_++, header);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::add_col(string_ref const header) {
    set_col_header(cols_++, header);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_text(
    int const row
  , int const col
  , string_ref const text
) {
    update_entry_(row + 1, col + 1, text);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::layout() {
    static auto const update = [](std::vector<cell_info>& values, int& sum, int const padding) {
        sum = padding;

        auto const n = values.size();
        for (size_t i = 0; i < n; ++i) {
            auto& info = values[i];

            info.offset = sum;
            sum += info.size;

            if (info.size) {
                sum += padding;
            }
        }
    };

    update(row_info_, total_h_, padding_row_);
    update(col_info_, total_w_, padding_col_);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_selection(int const row) {
    if (row >= rows_) {
        BK_TODO_FAIL();
    }

    selection_ = row;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::select_next() {
    if (rows_ <= 0) {
        BK_TODO_FAIL();
    }

    selection_ = (selection_ + 1) % rows_;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::select_prev() {
    if (rows_ <= 0) {
        BK_TODO_FAIL();
    }

    selection_ = (selection_ - 1) % rows_;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::clear() {
    selection_ = 0;
    rows_      = 0;
    cols_      = 0;

    total_w_ = 0;
    total_h_ = 0;

    row_info_.clear();
    col_info_.clear();

    title_.clear();
    
    for (auto& entry : entries_) {
        entry.second.clear();
    }
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::render(renderer& render) {
    auto const restore = render.restore_view();
    
    auto const title_rect = make_rect_size(
        pos_.x, pos_.y
      , total_w_ + border_size_ * 2
      , title_.actual_height() + title_border_size_
    );

    auto const border_rect = make_rect_size(
        pos_.x, pos_.y
      , total_w_ + border_size_ * 2
      , total_h_ + border_size_ * 2 + title_rect.height()
    );

    //
    // draw the outside border
    //
    render.set_draw_color(color_back_);
    render.draw_filled_rect(border_rect);

    //
    // draw the title text
    //
    title_.render(render, *face_
        , title_rect.left + border_size_
        , title_rect.top  + border_size_
    );

    auto const x0 = pos_.x + border_size_;
    auto const y0 = pos_.y + border_size_ + title_rect.height();

    auto const back_rect = make_rect_size(
        x0, y0, total_w_, total_h_
    );

    //
    // draw the even row background
    //
    render.set_draw_color(color_even_);
    render.draw_filled_rect(back_rect);

    //
    // draw the odd row background
    //
    render.set_draw_color(color_odd_);

    for (int i = 1; i <= rows_; i += 2) {
        auto const info = row_info_[i];
        auto const y = y0 + info.offset;
        auto const h = info.size;

        auto const rect = make_rect_size(x0, y, total_w_, h);
    
        render.draw_filled_rect(rect);
    }

    //
    // draw the actual text in the cells
    //
    for (auto const& entry : entries_) {
        auto const  row = entry.first.row;
        auto const  col = entry.first.col;
        auto const& txt = entry.second;

        auto const row_y = row_info_[row].offset;
        auto const col_x = col_info_[col].offset;

        auto const x = x0 + col_x;
        auto const y = y0 + row_y;

        auto changed = false;

        if (row == 0) {
            face_->set_color(make_color(200, 200, 0));
            changed = true;
        } else if (col == 0) {
            face_->set_color(make_color(0, 200, 200));
            changed = true;
        }

        txt.render(render, *face_, x, y);

        if (changed) {
            face_->set_color(make_color(200, 200, 200));
        }
    }

    face_->set_color(make_color(255, 255, 255)); //TODO
}

////////////////////////////////////////////////////////////////////////////////
// gui::list
////////////////////////////////////////////////////////////////////////////////
bkrl::gui::list::list(list&&) = default;
bkrl::gui::list::~list() = default;

bkrl::gui::list::list(font_face& face)
  : impl_ {std::make_unique<detail::list_impl>(face)}
{
}

bkrl::ipoint2 bkrl::gui::list::get_index_at(ipoint2 const p) const {
    return impl_->get_index_at(p);
}

int bkrl::gui::list::get_selection() const {
    return impl_->get_selection();
}

void bkrl::gui::list::set_title(string_ref const text) {
    impl_->set_title(text);
}

void bkrl::gui::list::set_col_header(int const col, string_ref const text) {
    impl_->set_col_header(col, text);
}

void bkrl::gui::list::set_row_header(int const row, string_ref const text) {
    impl_->set_row_header(row, text);
}

void bkrl::gui::list::set_position(ipoint2 const p) {
    impl_->set_position(p);
}

//void bkrl::gui::list::set_row_color(argb8 even, optional<argb8> odd) {
//    impl_->set_row_color(even, odd);
//}
//
//void bkrl::gui::list::set_selection_color(argb8 color) {
//    impl_->set_selection_color(color);
//}

void bkrl::gui::list::add_row(string_ref const header) {
    impl_->add_row(header);
}

void bkrl::gui::list::add_col(string_ref const header) {
    impl_->add_col(header);
}

//void bkrl::gui::list::set_col_width(int width) {
//}
//
//void bkrl::gui::list::set_row_height(int height ) {
//}

void bkrl::gui::list::set_text(int const row, int const col, string_ref const text) {
    impl_->set_text(row, col, text);
}

//void bkrl::gui::list::set_icon(int row, int col, int) {
//}
    
void bkrl::gui::list::set_selection(int const row) {
    impl_->set_selection(row);
}

void bkrl::gui::list::select_next() {
    impl_->select_next();
}

void bkrl::gui::list::select_prev() {
    impl_->select_prev();
}

void bkrl::gui::list::clear() {
    impl_->clear();
}

void bkrl::gui::list::layout() {
    impl_->layout();
}

void bkrl::gui::list::render(renderer& render) {
    impl_->render(render);
}

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

    void set_position(ipoint2 const p) {
        pos_ = p;
    }

    void set_title(string_ref const title) {
        title_.reset(*face_, title);
        row_w_ = std::max(row_w_, title_.actual_width()  + padding);
        row_h_ = std::max(row_h_, title_.actual_height() + padding);
    }

    void render(renderer& r) {
        auto const size = static_cast<int>(items_.size());

        if (size == 0) {
            return;
        }

        static auto color_background = make_color(50,  50,  50);
        static auto color_even       = make_color(13,  13,  13);
        static auto color_odd        = make_color(25,  25,  25);
        static auto color_highlight  = make_color(20, 20, 50);

        auto const x = pos_.x;
        auto const y = pos_.y;

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
    
    int index_at(int const x, int const y) const {
        auto const n = static_cast<int>(items_.size());

        if (n == 0) {
            return -1;
        }

        auto const left = pos_.x;
        auto const top  = pos_.y;

        if (x < left || y < top) {
            return -1;
        }

        auto const w = row_w_ + border * 2;
        auto const h = row_h_ * (n + 1) + border * 2;

        auto const right  = left + w;
        auto const bottom = top  + h;
        
        if (x >= right || y >= bottom) {
            return -1;
        }

        auto const i = (y - top) / row_h_;
        if (i < 1 || i > n) {
            return -1;
        }

        return i - 1;
    }

    item_id at(int const index) {
        BK_ASSERT(index >= 0 && index < size());
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

    void insert(bkrl::item_list const& items) {
        clear();

        for (auto&& iid : items) {
            insert(iid);
        }

        BK_ASSERT(!empty());
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

    void set_selection(int i) {
        BK_ASSERT(i < size());
        selection_ = i;
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

    ipoint2 pos_ = ipoint2 {0, 0};

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

void bkrl::gui::item_list::set_position(ipoint2 const p) {
    impl_->set_position(p);
}

void bkrl::gui::item_list::set_title(string_ref const title) {
    impl_->set_title(title);
}

void bkrl::gui::item_list::render(renderer& r) {
    impl_->render(r);
}

int bkrl::gui::item_list::index_at(int const x, int const y) const {
    return impl_->index_at(x, y);
}

bkrl::item_id bkrl::gui::item_list::at(int const index) {
    return impl_->at(index);
}

void bkrl::gui::item_list::insert(bkrl::item_list const& items) {
    impl_->insert(items);
}

void bkrl::gui::item_list::insert(item_id const id) {
    impl_->insert(id);
}

void bkrl::gui::item_list::clear() {
    impl_->clear();
}

void bkrl::gui::item_list::set_selection(int const i) {
    impl_->set_selection(i);
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
