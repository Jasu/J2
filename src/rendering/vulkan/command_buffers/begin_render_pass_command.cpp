#include "begin_render_pass_command.hpp"

#include "command_buffer_state.hpp"
#include "command_context.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"
#include "rendering/vulkan/rendering/render_pass.hpp"
#include "strings/string.hpp"

namespace {
  const VkClearValue clear_value{
    .color = {{
        0.0f, 0.0f, 0.0f, 1.0f
      }},
  };
}

namespace j::rendering::vulkan::inline command_buffers {
  begin_render_pass_command::begin_render_pass_command(state::node_insertion_context & context) {
    this->starts_epoch(epochs::render_pass);
    context.add_postcondition(is_in_render_pass = true);
  }

  void begin_render_pass_command::execute(command_context & context, state::reserved_resources_t &) const {
    auto & rc = *context.render_context;
    auto & fb = *rc.framebuffer;
    const VkRenderPassBeginInfo render_pass_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = rc.render_pass().vk_render_pass,
      .framebuffer = fb.vk_framebuffer,
      .renderArea = {
        { 0, 0 },
        fb.extent,
      },
      .clearValueCount = 1,
      .pClearValues = &clear_value,
    };
    vkCmdBeginRenderPass(context.vk_command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
  }

  strings::string begin_render_pass_command::name() const {
    return "Begin render pass";
  }
}
