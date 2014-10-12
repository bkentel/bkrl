#include "renderer.hpp"
#include "detail/sdl_application.i.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////

application::application(string_ref keymap, config const& cfg)
  : impl_ {std::make_unique<detail::application_impl>(keymap, cfg)}
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
renderer::set_translation_x(scalar const dx) {
    impl_->set_translation_x(dx);
}

void
renderer::set_translation_y(scalar const dy) {
    impl_->set_translation_y(dy);
}

void
renderer::set_scale_x(scalar const sx) {
    impl_->set_scale_x(sx);
}

void
renderer::set_scale_y(scalar const sy) {
    impl_->set_scale_y(sy);
}

vec2
renderer::get_scale() const {
    return impl_->get_scale();
}

vec2
renderer::get_translation() const {
    return impl_->get_translation();
}

void
renderer::draw_texture(texture const& tex, scalar x, scalar y) {
    impl_->draw_texture(tex, x, y);
}

void
renderer::draw_texture(texture const& tex, rect const src, rect const dst) {
    impl_->draw_texture(tex, src, dst);
}

texture
renderer::create_texture(string_ref filename) {
    return impl_->create_texture(filename);
}

texture
renderer::create_texture(uint8_t* buffer, int width, int height) {
    return impl_->create_texture(buffer, width, height);
}

texture
renderer::create_texture(int width, int height) {
    return impl_->create_texture(width, height);
}

void
renderer::delete_texture(texture& tex) {
    impl_->delete_texture(tex);
}

void
renderer::update_texture(texture& tex, void* data, int pitch, int x, int y, int w, int h) {
    impl_->update_texture(tex, data, pitch, x, y, w, h);
}

void
renderer::set_color_mod(texture& tex, uint8_t r, uint8_t g, uint8_t b) {
    impl_->set_color_mod(tex, r, g, b);
}

void
renderer::set_alpha_mod(texture & tex, uint8_t a) {
    impl_->set_alpha_mod(tex, a);
}

void
renderer::set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    impl_->set_draw_color(r, g, b, a);
}

void
renderer::draw_filled_rect(rect bounds) {
    impl_->draw_filled_rect(bounds);
}

////////////////////////////////////////////////////////////////////////////////
