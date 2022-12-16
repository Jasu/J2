#include "set_viewport_command.hpp"

#include "command_buffer_state.hpp"
#include "command_context.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"
#include "strings/format.hpp"

#include <vulkan/vulkan.h>

namespace j::rendering::vulkan::inline command_buffers {
  set_viewport_command::set_viewport_command(state::node_insertion_context & context, const g::rect_u16 & region)
    : m_region(region)
  {
    this->is_bound_to_epoch(epochs::in_render_pass);
    this->starts_epoch(epochs::draw_command);
    context.add_postcondition(viewport = m_region);
  }

  void set_viewport_command::execute(command_context & context, state::reserved_resources_t &) const {
    auto ext = context.framebuffer().extent;
    u32_t width = ::j::min(ext.width - m_region.left(), m_region.width());
    u32_t height = ::j::min(ext.height - m_region.top(), m_region.height());
    const VkRect2D scissor{
      .offset = { m_region.left(), m_region.top() },
      .extent = { width, height },
    };
    vkCmdSetScissor(context.vk_command_buffer, 0, 1, &scissor);

    const VkViewport viewport{
      .x = -(float)ext.width + m_region.left(),
      .y = -(float)ext.height + m_region.top(),
      .width = ext.width * 2.0f,
      .height = ext.height * 2.0f,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    };
    vkCmdSetViewport(context.vk_command_buffer, 0, 1, &viewport);
  }

  strings::string set_viewport_command::name() const {
    return strings::format("Set viewport to {}", m_region);
  }
}
