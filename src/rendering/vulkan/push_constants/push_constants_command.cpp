#include "rendering/vulkan/push_constants/push_constants_command.hpp"

#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "rendering/vulkan/rendering/pipeline.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "strings/string.hpp"
#include "rendering/vulkan/push_constants/push_constant_state.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
    push_constants_command::push_constants_command(
      state::node_insertion_context & context,
      detail::copy_tag,
      u8_t push_constant_offset,
      u8_t size,
      const void * data,
      const void * push_constant_tag
    ) : m_push_constant_offset(push_constant_offset),
        m_owns_data(true),
        m_inline_data(size < sizeof(void*))
    {
      this->is_bound_to_current_epoch(command_buffers::epochs::pipeline_binding);
      J_ASSERT_NOT_NULL(size, data);
      if (m_inline_data) {
        ::j::memcpy(&m_data, data, size);
      } else {
        m_data = ::j::allocate(size);
        ::j::memcpy(m_data, data, size);
      }
      context.add_postcondition(push_constant[push_constant_offset] = push_constant_tag);
    }

    push_constants_command::push_constants_command(
      state::node_insertion_context & context,
      detail::no_copy_tag,
      u8_t push_constant_offset,
      const void * data
    ) : m_data(const_cast<void*>(data)),
        m_push_constant_offset(push_constant_offset),
        m_owns_data(false),
        m_inline_data(false)
    {
      this->is_bound_to_current_epoch(command_buffers::epochs::pipeline_binding);
      J_ASSERT_NOT_NULL(m_data);
      context.add_postcondition(push_constant[push_constant_offset] = m_data);
    }

    push_constants_command::~push_constants_command() {
      if (m_owns_data && !m_inline_data) {
        ::j::free(m_data);
      }
    }

    void push_constants_command::execute(command_buffers::command_context & context, state::reserved_resources_t &) const {
      const auto & pipeline = context.render_context->get(context.current_pipeline);
      const VkPushConstantRange & range = pipeline.get_push_constant_range(m_push_constant_offset);
      J_ASSERT_NOT_NULL(range.stageFlags, range.size);
      vkCmdPushConstants(context.vk_command_buffer,
                         pipeline.vk_pipeline_layout,
                         range.stageFlags,
                         range.offset,
                         range.size,
                         m_inline_data ? &m_data : m_data);
    }

    strings::string push_constants_command::name() const {
      return "Push constants";
    }
  }
}
