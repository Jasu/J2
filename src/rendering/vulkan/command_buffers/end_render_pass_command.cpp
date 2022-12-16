#include "end_render_pass_command.hpp"
#include "command_context.hpp"
#include "command_buffer_state.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  end_render_pass_command::end_render_pass_command(state::node_insertion_context & context) {
    this->starts_epoch(epochs::render_pass);
    context.add_postcondition(is_in_render_pass = false);
  }

  void end_render_pass_command::execute(command_context & context, state::reserved_resources_t &) const {
    context.reset_current_pipeline();
    vkCmdEndRenderPass(context.vk_command_buffer);
  }

  strings::string end_render_pass_command::name() const {
    return "End render pass";
  }
}
