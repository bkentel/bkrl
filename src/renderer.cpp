#include "renderer.hpp"
#include "detail/sdl_application.i.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////

application::application(string_ref keymap)
  : impl_ {std::make_unique<detail::application_impl>(keymap)}
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

void
renderer::draw_texture(texture const& tex, scalar x, scalar y) {
    impl_->draw_texture(tex, x, y);
}

void
renderer::draw_texture(texture const& tex, rect const src, rect const dst) {
    impl_->draw_texture(tex, src, dst);
}

void
renderer::draw_tile(
    tile_sheet const& sheet
  , unsigned const ix
  , unsigned const iy
  , scalar   const x
  , scalar   const y
) {
    impl_->draw_tile(sheet, ix, iy, x, y);
}

texture
renderer::create_texture(string_ref filename) {
    return impl_->create_texture(filename);
}

texture
renderer::create_texture(uint8_t* buffer, int width, int height) {
    return impl_->create_texture(buffer, width, height);
}

void
renderer::delete_texture(texture& tex) {
    impl_->delete_texture(tex);
}

////////////////////////////////////////////////////////////////////////////////
