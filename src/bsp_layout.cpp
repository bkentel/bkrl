#include "bsp_layout.hpp"
#include "detail/bsp_layout.i.hpp"

using namespace bkrl;

bsp_layout::~bsp_layout() = default;

bsp_layout::bsp_layout(bsp_layout&& other)
  : impl_ {std::move(other.impl_)}
{
}

bsp_layout::bsp_layout() = default;

bsp_layout::bsp_layout(
    random::generator&    gen
  , params_t       const& params
  , split_callback const& on_split
  , room_callback  const& on_room_gen
  , grid_region    const& reserve
)
  : impl_ {std::make_unique<detail::bsp_layout_impl>(
        gen, params, on_split, on_room_gen, reserve
    )}
{
}

bsp_layout bsp_layout::generate(
    random::generator&    gen
  , split_callback const& on_split
  , room_callback  const& on_room_gen
  , params_t       const& params
  , grid_region    const& reserve
) {
    return bsp_layout {gen, params, on_split, on_room_gen, reserve};
}

void bsp_layout::connect(random::generator& gen, connect_callback on_connect) {
    impl_->connect(gen, on_connect);
}
