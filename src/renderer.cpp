#include "renderer.hpp"
#include "detail/sdl_application.i.hpp"

using namespace bkrl;

using bkrl::renderer;

////////////////////////////////////////////////////////////////////////////////
// application
////////////////////////////////////////////////////////////////////////////////
application::application(keymap const& map, config const& cfg)
  : impl_ {std::make_unique<detail::application_impl>(map, cfg)}
{
}

application::~application() = default;

application::handle_t
application::handle() const{
    return impl_->handle();
}

bool
application::is_running() const {
    return impl_->is_running();
}

bool
application::has_events() const {
    return impl_->has_events();
}

int
application::client_width() const {
    return impl_->client_width();
}

int
application::client_height() const {
    return impl_->client_height();
}

void
application::do_one_event() {
    impl_->do_one_event();
}

void
application::do_all_events() {
    impl_->do_all_events();
}

void
application::sleep(int const ms) const {
    impl_->sleep(ms);
}

bkrl::key_modifier
application::get_kb_mods() const {
    return impl_->get_kb_mods();
}

void
application::on_char(char_sink sink) {
    impl_->on_command(sink);
}

void
application::on_command(command_sink sink) {
    impl_->on_command(sink);
}

void
application::on_close(close_sink sink) {
    impl_->on_close(sink);
}

void
application::on_resize(resize_sink sink) {
    impl_->on_resize(sink);
}

void
application::on_mouse_move(mouse_move_sink sink) {
    impl_->on_mouse_move(sink);
}

void
application::on_mouse_button(mouse_button_sink sink) {
    impl_->on_mouse_button(sink);
}

void
application::on_mouse_wheel(mouse_wheel_sink sink) {
    impl_->on_mouse_wheel(sink);
}

////////////////////////////////////////////////////////////////////////////////
// renderer
////////////////////////////////////////////////////////////////////////////////
renderer::renderer(application const& app)
  : impl_ {std::make_unique<detail::renderer_impl>(app)}
{
}

renderer::~renderer() = default;

renderer::handle_t
renderer::handle() const {
    return impl_->handle();
}

void
renderer::clear() {
    impl_->clear();
}

void
renderer::present() {
    impl_->present();
}

void
renderer::set_translation_x(trans_t const dx) {
    impl_->set_translation_x(dx);
}

void
renderer::set_translation_y(trans_t const dy) {
    impl_->set_translation_y(dy);
}

void
renderer::set_scale_x(scale_t const sx) {
    impl_->set_scale_x(sx);
}

void
renderer::set_scale_y(scale_t const sy) {
    impl_->set_scale_y(sy);
}

void
renderer::set_scale(scale_t const sx, scale_t const sy) {
    impl_->set_scale(sx, sy);
}

renderer::scale_vec
renderer::get_scale() const {
    return impl_->get_scale();
}

renderer::trans_vec
renderer::get_translation() const {
    return impl_->get_translation();
}

void
renderer::draw_texture(texture const& tex, pos_t const x, pos_t const y) {
    impl_->draw_texture(tex, x, y);
}

void
renderer::draw_texture(texture const& tex, rect_t const src, rect_t const dst) {
    impl_->draw_texture(tex, src, dst);
}

texture
renderer::create_texture(path_string_ref const filename) {
    return impl_->create_texture(filename);
}

texture
renderer::create_texture(void const* const buffer, tex_coord_i const width, tex_coord_i const height) {
    return impl_->create_texture(buffer, width, height);
}

texture
renderer::create_texture(tex_coord_i const width, tex_coord_i const height) {
    return impl_->create_texture(width, height);
}

void
renderer::delete_texture(texture& tex) {
    impl_->delete_texture(tex);
}

void
renderer::update_texture(texture& tex, void const* data, int const pitch, int const x, int const y, int const w, int const h) {
    impl_->update_texture(tex, data, pitch, x, y, w, h);
}

bkrl::argb8
bkrl::renderer::get_color_mod(texture const& tex) const {
    return impl_->get_color_mod(tex);
}

void
renderer::set_color_mod(texture& tex) {
    static auto const clear_color = make_color(255, 255, 255);
    set_color_mod(tex, clear_color);
}

void
renderer::set_color_mod(texture& tex, color3b const color) {
    impl_->set_color_mod(tex, color.r, color.g, color.b);
}

void
renderer::set_color_mod(texture& tex, color4b const color) {
    impl_->set_color_mod(tex, color.r, color.g, color.b, color.a);
}

void
renderer::set_alpha_mod(texture & tex, uint8_t const a) {
    impl_->set_alpha_mod(tex, a);
}

void
renderer::set_draw_color() {
    static auto const clear_color = make_color(0, 0, 0);
    set_draw_color(clear_color);
}

void
renderer::set_draw_color(color4b const color) {
    impl_->set_draw_color(color.r, color.g, color.b, color.a);
}

void
renderer::set_draw_color(color3b const color) {
    impl_->set_draw_color(color.r, color.g, color.b);
}

void
renderer::draw_filled_rect(rect_t const bounds) {
    impl_->draw_filled_rect(bounds);
}

////////////////////////////////////////////////////////////////////////////////
