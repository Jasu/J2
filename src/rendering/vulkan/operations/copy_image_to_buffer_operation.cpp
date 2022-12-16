#include "rendering/vulkan/operations/copy_image_to_buffer_operation.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/images/image_source_manager.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::operations {
  namespace i = j::rendering::images;
  copy_image_to_buffer_operation::copy_image_to_buffer_operation(
    state::node_insertion_context & context,
    i::image_source_key image,
    i::image_buffer_view target,
    const state::postcondition_initializer & postcondition
  ) : m_image_source_key(image),
      m_target(target)
  {
    J_ASSERT_NOT_NULL(m_image_source_key, m_target);
    this->is_bound_to_epoch(command_buffers::epochs::before_render_pass);
    context.add_postcondition(postcondition);
  }

  void copy_image_to_buffer_operation::execute(command_context & context, state::reserved_resources_t &) const {
    i::image_source_manager::instance.copy_to(*context.render_context, m_target, m_image_source_key);
  }

  strings::string copy_image_to_buffer_operation::name() const {
    return strings::format("Load {} to buffer",
                           j::rendering::images::image_source_manager::instance.describe(m_image_source_key));
  }
}
