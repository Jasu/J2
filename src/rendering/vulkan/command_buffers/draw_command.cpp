#include "draw_command.hpp"

#include "command_context.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  void draw_command::execute(command_context & context, state::reserved_resources_t &) const {
    vkCmdDraw(context.vk_command_buffer, m_vertex_count, m_instance_count, m_first_vertex, m_first_instance);
  }

  strings::string draw_command::name() const {
    return "Draw";
  }
}
