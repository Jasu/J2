#include "ui/rendering/element_frame.hpp"
#include "rendering/renderers/box/box_renderer.hpp"
#include "ui/rendering/context.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "mem/shared_ptr.hpp"
#include "ui/styling/style.hpp"

namespace j::ui::inline rendering {
  void element_frame::render(context & ctx, const g::rect_u16 & region,
                             const g::perimeter_u8 & border_size, const style & s) const
  {
    if (!s.has_background() && !s.has_border()) {
      return;
    }

    m_box.assign_from({
      g::rect_i16{0, 0, region.width(), region.height()},
      s.background_color,
      s.border_color_left,
      s.border_color_right,
      s.border_color_top,
      s.border_color_bottom,
      s.corner_radius,
      border_size});
    ctx.builder->set_viewport(region);
    ctx.box_renderer->draw_box(*ctx.builder, m_box);
  }
}
