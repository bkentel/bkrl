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

    int add_row(string_ref header);
    int add_col(string_ref header);

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
        int16_t size;
        int16_t offset;
    };

    void update_entry_(int row, int col, string_ref text);
    void change_selection_(int delta);

    inline irect get_border_rect_() const noexcept {
        return make_rect_size<int>(
            pos_.x
          , pos_.y
          , total_w_ + border_size_ * 2
          , total_h_ + border_size_ * 2 + title_.actual_height() + title_border_size_
        );
    }

    inline irect get_title_rect_() const noexcept {
        return make_rect_size<int>(
            pos_.x
          , pos_.y
          , total_w_ + border_size_ * 2
          , title_.actual_height() + title_border_size_
        );
    }

    inline irect get_list_rect_() const noexcept {
        return make_rect_size<int>(
            pos_.x + border_size_
          , pos_.y + border_size_ + title_.actual_height() + title_border_size_
          , total_w_
          , total_h_
        );
    }

    font_face* face_      = nullptr;
    int        selection_ = 0;
    ipoint2    pos_       = ipoint2 {0, 0};
    int        rows_      = 0;
    int        cols_      = 0;
    int16_t    total_w_   = 0;
    int16_t    total_h_   = 0;

    std::vector<cell_info> row_info_;
    std::vector<cell_info> col_info_;

    transitory_text_layout title_;
    
    boost::container::flat_map<key_t, transitory_text_layout, std::less<>> entries_;

    int16_t border_size_        = 4;
    int16_t title_border_size_  = 4;
    int16_t padding_col_        = 8;
    int16_t padding_row_        = 0;

    argb8 color_even_ = argb8 {255, 30, 30, 30};
    argb8 color_odd_  = argb8 {255, 40, 40, 40};
    argb8 color_sel_  = argb8 {255, 100, 100, 20};
    argb8 color_back_ = argb8 {255, 80, 60, 60};
};

bkrl::ipoint2 bkrl::gui::detail::list_impl::get_index_at(ipoint2 const p) const {
    auto const rect = get_list_rect_();
    if (!bkrl::intersects(rect, p)) {
        return {-1, -1};
    }
    
    auto const y = (p.y - rect.top);
    auto const x = (p.x - rect.left);

    auto row = 0;
    for (auto const& info : row_info_) {
        if (y < info.offset + info.size) { break; }
        row++;
    }

    auto col = 0;
    for (auto const& info : col_info_) {
        if (x < info.offset + info.size) { break; }
        col++;
    }

    return {col - 1, row - 1};
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

            return result.first->second;
        } else {
            it->second.reset(*face_, text);
            return it->second;
        }
    }();

    auto const ri = static_cast<size_t>(row);
    auto const ci = static_cast<size_t>(col);

    if (ri >= row_info_.size()) {
        row_info_.resize(ri + 1, cell_info {0, 0});
    }

    if (ci >= col_info_.size()) {
        col_info_.resize(ci + 1, cell_info {0, 0});
    }

    auto& row_size = row_info_[row].size;
    auto& col_size = col_info_[col].size;

    row_size = static_cast<int16_t>(face_->line_gap());
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
int bkrl::gui::detail::list_impl::add_row(string_ref const header) {
    set_row_header(rows_++, header);
    return rows_ - 1;
}

//--------------------------------------------------------------------------
int bkrl::gui::detail::list_impl::add_col(string_ref const header) {
    set_col_header(cols_++, header);
    return cols_ - 1;
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
    static auto const update = [](std::vector<cell_info>& values, int16_t& sum, int16_t const padding) {
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
void bkrl::gui::detail::list_impl::change_selection_(int const delta) {
    BK_ASSERT(rows_ > 0);

    selection_ = (selection_ + delta) % rows_;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::set_selection(int const row) {
    BK_ASSERT(row >= 0 && row < rows_);

    selection_ = row;
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::select_next() {
    change_selection_(1);
}

//--------------------------------------------------------------------------
void bkrl::gui::detail::list_impl::select_prev() {
    change_selection_(rows_ - 1);
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

    auto const back_rect = make_rect_size<int>(
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

        auto const rect = make_rect_size<int>(x0, y, total_w_, h);
    
        render.draw_filled_rect(rect);
    }

    //
    // draw the selection
    //
    render.set_draw_color(color_sel_);
    {
        auto const info = row_info_[selection_ + 1];
        auto const y = y0 + info.offset;
        auto const h = info.size;

        auto const rect = make_rect_size<int>(x0, y, total_w_, h);
    
        render.draw_filled_rect(rect);
    }

    //
    // draw the actual text in the cells
    //
    for (auto const& entry : entries_) {
        auto const  row = entry.first.row;
        auto const  col = entry.first.col;
        auto const& txt = entry.second;

        if (txt.empty()) {
            continue;
        }

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

    render.set_draw_color(color_back_);
    for (size_t i = 1; i < col_info_.size(); ++i) {
        auto const& col = col_info_[i];
        
        if (col.size <= 0) { continue; }

        auto const rect = make_rect_size<int>(
            x0 + col.offset - padding_col_ / 2
            , y0
            , 1
            , total_h_
        );

        render.draw_filled_rect(rect);
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

int bkrl::gui::list::add_row(string_ref const header) {
    return impl_->add_row(header);
}

int bkrl::gui::list::add_col(string_ref const header) {
    return impl_->add_col(header);
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
    enum {
        col_itm_name    = 0
      , col_itm_weight  = 1
      , col_itm_type    = 2
      , col_itm_details = 3

      , col_eq_slot    = 0
      , col_eq_name    = 1
      , col_eq_weight  = 2
      , col_eq_details = 3
    };

    item_list_impl(
        font_face&              face
      , item_definitions const& item_defs
      , item_store       const& items
      , message_map      const& messages
    )
      : list_       {face}
      , item_defs_  {&item_defs}
      , item_store_ {&items}
      , messages_   {&messages}
    {
        clear();
    }

    void set_position(ipoint2 const p) {
        list_.set_position(p);
    }

    void set_title(string_ref const title) {
        list_.set_title(title);
    }

    void render(renderer& r) {
        if (items_.empty()) {
            return;
        }

        list_.render(r);
    }
    
    int index_at(ipoint2 const p) const {
        auto const where = list_.get_index_at(p);
        return where.y;
    }

    item_id at(int const index) const {        
        return items_[index];
    }

    void insert_item(item_id const id) {
        auto const& istore = *item_store_;
        auto const& idefs  = *item_defs_;
        auto const& msgs   = *messages_;

        insert_item_(id, istore, idefs, msgs);
    }

    void insert_item(item_collection const& items) {
        clear();

        auto const& istore = *item_store_;
        auto const& idefs  = *item_defs_;
        auto const& msgs   = *messages_;

        list_.add_col(msgs[message_type::header_items]);
        list_.add_col("Weight");
        list_.add_col("Type");
        list_.add_col("Details");

        items.for_each_item([&](item_id const itm) {
            insert_item_(itm, istore, idefs, msgs);
        });

        list_.layout();

        BK_ASSERT(!empty());
    }

    void insert_item_(item_id const id, item_store const& istore, item_definitions const& idefs, message_map const& msgs) {
        auto const& itm    = istore[id];
        auto const& name   = itm.name(idefs);
        auto const& type   = to_string(msgs, itm.type);
        auto const& weight = std::to_string(itm.weight(idefs));
        auto const& info   = itm.short_description(msgs);

        name_buffer_.clear();
        name_buffer_.push_back(prefix_++);

        auto const row = list_.add_row(name_buffer_);

        list_.set_text(row, col_itm_name,    name);
        list_.set_text(row, col_itm_weight,  weight);
        list_.set_text(row, col_itm_type,    type);
        list_.set_text(row, col_itm_details, info);
        
        items_.push_back(id);
    }

    void insert_equip(item_collection const& items) {
        using msg = message_type;
        using eqs = equip_slot;

        auto const& istore = *item_store_;
        auto const& idefs  = *item_defs_;
        auto const& msgs   = *messages_;

        auto const slot_count = enum_value(equip_slot::enum_size);
        
        list_.add_col(msgs[msg::header_slot]);
        list_.add_col(msgs[msg::header_items]);
        list_.add_col("Weight");
        list_.add_col("Details");

        name_buffer_.clear();
        name_buffer_.push_back(prefix_);

        for (int i = 0; i < slot_count - 1; ++i) {
            list_.add_row(name_buffer_);

            name_buffer_[0]++;
            prefix_++;
        }
        
        auto const set_slot_name = [&](equip_slot const slot, message_type const hdr) {
            list_.set_text(
                enum_value(slot) - 1
              , col_eq_slot
              , msgs[hdr]
            );
        };

        set_slot_name(eqs::head, msg::eqslot_head);
        set_slot_name(eqs::arms_upper, msg::eqslot_arms_upper);
        set_slot_name(eqs::arms_lower, msg::eqslot_arms_lower);
        set_slot_name(eqs::hands, msg::eqslot_hands);
        set_slot_name(eqs::chest, msg::eqslot_chest);
        set_slot_name(eqs::waist, msg::eqslot_waist);
        set_slot_name(eqs::legs_upper, msg::eqslot_legs_upper);
        set_slot_name(eqs::legs_lower, msg::eqslot_legs_lower);
        set_slot_name(eqs::feet, msg::eqslot_feet);
        set_slot_name(eqs::finger_left, msg::eqslot_finger_left);
        set_slot_name(eqs::finger_right, msg::eqslot_finger_right);
        set_slot_name(eqs::neck, msg::eqslot_neck);
        set_slot_name(eqs::back, msg::eqslot_back);
        set_slot_name(eqs::hand_main, msg::eqslot_hand_main);
        set_slot_name(eqs::hand_off, msg::eqslot_hand_off);
        set_slot_name(eqs::ammo, msg::eqslot_ammo);

        items_.resize(slot_count, item_id {0});

        items.for_each_item([&](item_id const iid) {
            auto const& itm    = istore[iid];
            auto const& name   = itm.name(idefs);
            auto const& info   = itm.short_description(msgs);
            auto const& weight = std::to_string(itm.weight(idefs));

            BK_ASSERT_DBG(itm.is_equippable(idefs));

            auto const slots = itm.equip_slots(idefs);

            for (int i = 1; i < slot_count - 1; ++i) {
                if (!slots.test(i)) { continue; }

                auto const row = i - 1;

                list_.set_text(row, col_eq_name,    name);
                list_.set_text(row, col_eq_weight,  weight);
                list_.set_text(row, col_eq_details, info);
                items_[row] = iid;
            }
        });

        list_.layout();
    }

    void clear() {
        list_.clear();
        items_.clear();
        prefix_ = 'a';
    }

    void set_selection(int const i) {
        list_.set_selection(i);
    }

    void select_next() {
        list_.select_next();
    }

    void select_prev() {
        list_.select_prev();
    }

    int get_selection() const noexcept {
        return list_.get_selection();
    }

    int size() const noexcept {
        return items_.size();
    }
    
    bool empty() const noexcept {
        return items_.empty();
    }
private:
    list list_;

    item_definitions const* item_defs_  = nullptr;
    item_store       const* item_store_ = nullptr;
    message_map      const* messages_   = nullptr;

    char prefix_ = 'a';

    utf8string           name_buffer_;
    std::vector<item_id> items_;
};

bkrl::gui::item_list::item_list(item_list&&) = default;
bkrl::gui::item_list::~item_list() = default;

bkrl::gui::item_list::item_list(
    font_face&              face
  , item_definitions const& item_defs
  , item_store       const& items
  , message_map      const& messages
)
  : impl_ {std::make_unique<detail::item_list_impl>(face, item_defs, items, messages)}
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

int bkrl::gui::item_list::get_index_at(ipoint2 const p) const {
    return impl_->index_at(p);
}

bkrl::item_id bkrl::gui::item_list::at(int const index) {
    return impl_->at(index);
}

void bkrl::gui::item_list::insert(item_collection const& items) {
    impl_->insert_item(items);
}

void bkrl::gui::item_list::insert(item_id const id) {
    impl_->insert_item(id);
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

int bkrl::gui::item_list::get_selection() const {
    return impl_->get_selection();
}

int bkrl::gui::item_list::size() const noexcept {
    return impl_->size();
}

bool bkrl::gui::item_list::empty() const noexcept {
    return impl_->empty();
}

////////////////////////////////////////////////////////////////////////////////
// gui::equip_list
////////////////////////////////////////////////////////////////////////////////
void bkrl::gui::equip_list::insert(item_collection const& items) {
    impl_->insert_equip(items);
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
bkrl::gui::message_log::render(renderer& r) {
    auto const back_color = make_color(
        50, 50, 50, (faded_ ? 50 : 240)
    );

    auto const restore = r.restore_view();

    //
    // draw background
    //

    r.set_draw_color(back_color);
    r.draw_filled_rect(bounds_);

    //
    // draw text
    //
    auto const line_h = font_face_->line_gap();

    auto const beg = (front_ < visible_lines_)
      ? (line_count + (front_ - visible_lines_))
      : (front_ - visible_lines_);

    auto const end = front_;

    auto const x = bounds_.left;
    auto       y = bounds_.top;

    for (auto i = beg; i != end; i = (i + 1) % line_count) {
        auto const& line = lines_[i];
        if (line.empty()) {
            continue;
        }

        line.render(r, *font_face_, x, y);
        y += line_h;
    }
}

//--------------------------------------------------------------------------
void bkrl::gui::message_log::set_bounds(irect const bounds) {
    bounds_ = bounds;

    auto const line_h = font_face_->line_gap();
    bounds_.bottom = bounds_.top + line_h * 3;

    visible_lines_ = bounds_.height() / line_h;
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::print_line_(format_t& format) {
    make_line_(boost::str(format));
}

//--------------------------------------------------------------------------
void
bkrl::gui::message_log::make_line_(string_ref const str) {
    constexpr auto max_w = int16_t {1000};
    auto const     max_h = static_cast<int16_t>(font_face_->line_gap());

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

////////////////////////////////////////////////////////////////////////////////
// gui::map_inspect
////////////////////////////////////////////////////////////////////////////////
void bkrl::gui::map_inspect::reset(font_face& face, string_ref const msg) {
    font_face_ = &face;

    text_.reset(face, msg, 320);

    render_w_ = text_.actual_width();
    render_h_ = text_.actual_height();

    //adjust the height to the next full line height
    auto const h = face.line_gap();
    auto const n = render_h_ % h;

    if (n) {
        render_h_ += (h - n);
    }
}

void bkrl::gui::map_inspect::render(renderer& r) {
    auto const color_text_background = make_color(50, 50, 50, 240);
    auto const color_text_border     = make_color(180, 100, 0, 255);
    auto constexpr padding = 4;
    auto constexpr border = 2;

    if (!visible_) {
        return;
    }

    if (text_.empty()) {
        return;
    }

    auto const restore = r.restore_view();

    auto x = render_x_ - padding;
    auto y = render_y_ - padding - (render_h_ + padding);
    auto w = render_w_ + padding * 2;
    auto h = render_h_ + padding * 2;

    //TODO adjust for border size too; cleanup for readability

    if (view_w_ && view_h_) {
        if (y + h > view_h_) {
            y += view_h_ - (y + h);
        }

        if (x + w > view_w_) {
            x += view_w_ - (x + w);
        }

        if (y < 0) { y = 0; }
        if (x < 0) { x = 0; }
    }

    r.set_draw_color(color_text_background);
    r.draw_filled_rect(make_rect_size(x, y, w, h));

    r.set_draw_color(color_text_border);
    r.draw_filled_rect(make_rect_size(
        x - border
      , y - border
      , w + border * 2
      , border));
    
    r.draw_filled_rect(make_rect_size(
        x - border
      , y + h
      , w + border * 2
      , border));
    
    r.draw_filled_rect(make_rect_size(x - border, y, border, h));
    r.draw_filled_rect(make_rect_size(x + w, y, border, h));
    
    text_.render(r, *font_face_, x + padding, y + padding);
}
