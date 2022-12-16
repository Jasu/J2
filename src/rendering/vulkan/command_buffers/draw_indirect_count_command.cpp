#include "draw_indirect_count_command.hpp"

#include "command_context.hpp"
#include "rendering/vulkan/rendering/indirect_buffer.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  void draw_indirect_count_command::execute(command_context & context, state::reserved_resources_t &) const {
    auto & indirect_buffer = context.render_context->get(m_buffer);
    context.render_context->device().vkCmdDrawIndirectCountKHR(context.vk_command_buffer,
                                                               indirect_buffer.vk_buffer, 4,
                                                               indirect_buffer.vk_buffer, 0,
                                                               indirect_buffer.size(),
                                                               sizeof(VkDrawIndirectCommand));
  }

  strings::string draw_indirect_count_command::name() const {
    return "Draw indirect count";
  }

  draw_indirect_count_command::~draw_indirect_count_command() {
  }
}
