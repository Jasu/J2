#include "rendering/renderers/text/text_renderer_string.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"

namespace j::rendering::renderers::text {
  text_renderer_string::text_renderer_string(
    text_renderer * renderer,
    geometry::vec2f pos,
    mem::shared_ptr<fonts::shaping::shaped_string> string,
    colors::rgba8 color)
    : m_renderer(renderer),
      m_color(color),
      m_position(pos),
      m_string(static_cast<mem::shared_ptr<fonts::shaping::shaped_string> &&>(string))
  {
    J_ASSERT_NOT_NULL(m_renderer, m_string);
  }

  text_renderer_string::~text_renderer_string() {
    if (m_allocation) {
      m_allocation.release();
    }
  }
}
