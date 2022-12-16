#include "rendering/vulkan/push_constants/push_special_constant_command.hpp"
#include "rendering/vulkan/push_constants/push_constant_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"
#include "rendering/vulkan/rendering/pipeline.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
    push_special_constant_command::push_special_constant_command(
      state::node_insertion_context & context,
      u32_t push_constant_index, special_push_constant type)
      : m_push_constant_index(push_constant_index),
        m_type(type)
    {
      context.add_postcondition(push_constant[push_constant_index] = type);
    }

    void push_special_constant_command::execute(command_buffers::command_context & context, state::reserved_resources_t &) const {
      float data[2];
      const VkExtent2D & extent = context.framebuffer().extent;
      switch (m_type) {
      case special_push_constant::viewport_size_vec2:
        data[0] = extent.width;
        data[1] = extent.height;
        break;
      case special_push_constant::inverse_viewport_size_vec2:
        data[0] = 1.0f / (float)extent.width;
        data[1] = 1.0f / (float)extent.height;
        break;
      case special_push_constant::max: J_UNREACHABLE();
      }
      vkCmdPushConstants(context.vk_command_buffer,
                         context.render_context->get(context.current_pipeline).vk_pipeline_layout,
                         VK_SHADER_STAGE_VERTEX_BIT, m_push_constant_index,
                         sizeof(float) * 2, data);
    }

    strings::string push_special_constant_command::name() const {
      switch (m_type) {
      case special_push_constant::viewport_size_vec2:
        return "Viewport size (vec2)";
      case special_push_constant::inverse_viewport_size_vec2:
        return "Inverse viewport size (vec2)";
      case special_push_constant::max: J_UNREACHABLE();
      }
    }
  }
}
