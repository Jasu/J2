#include "rendering/vulkan/operations/copy_to_vertex_buffer_operation.hpp"

#include "rendering/vulkan/context/render_context.hpp"
#include "strings/string.hpp"
#include "rendering/vertex_data/vertex_data_source_manager.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"

namespace j::rendering::vulkan::operations {

  copy_to_vertex_buffer_operation::copy_to_vertex_buffer_operation(
    state::node_insertion_context & context,
    vertex_data::vertex_data_source_key source,
    vertex_data::vertex_buffer_view target,
    const state::postcondition_initializer & postcondition
  ) : m_source_key(source),
      m_target(target)
  {
    J_ASSERT_NOT_NULL(m_source_key, m_target);
    this->is_bound_to_epoch(command_buffers::epochs::before_render_pass);
    context.add_postcondition(postcondition);
  }

  void copy_to_vertex_buffer_operation::execute(command_context & context, state::reserved_resources_t &) const {
    vertex_data::vertex_data_source_manager::instance.copy_to(*context.render_context, m_target, m_source_key);
  }

  strings::string copy_to_vertex_buffer_operation::name() const {
    return "Copy to vertex buffer";
  }
}
