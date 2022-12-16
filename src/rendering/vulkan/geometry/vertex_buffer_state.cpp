#include "rendering/vulkan/geometry/vertex_buffer_state.hpp"
#include "strings/string.hpp"
#include "rendering/vulkan/operations/copy_to_vertex_buffer_operation.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"

namespace j::rendering::vulkan::geometry {
  namespace s = state;

  bool vertex_buffer_contents_var::can_transition(
    const s::transition_context &,
      vertex_data::vertex_data_source_key from,
      vertex_data::vertex_data_source_key to) const
  {
    return from != to && !to.empty();
  }

  s::condition_group * vertex_buffer_contents_var::transition(
      s::transition_context & context,
      vertex_buffer_ref ref,
      [[maybe_unused]] vertex_data::vertex_data_source_key from,
      vertex_data::vertex_data_source_key to) const
  {
    J_ASSERT_NOT_NULL(ref);
    J_ASSERT(can_transition(context, from, to), "Cannot transition buffer contents.");
    auto & buf = context.context.render_context.get(ref);
    return ::new operations::copy_to_vertex_buffer_operation(
      context.context,
      to,
      vertex_data::vertex_buffer_view(buf.info(), buf.buffer_data()),
      (*this)(ref) = to
    );
  }

  strings::string vertex_buffer_contents_var::describe_precondition(
    resources::resource_wrapper *, uptr_t, const s::precondition_instance &) const
  { return "Vertex buffer must contain"; }

  strings::string vertex_buffer_contents_var::describe_postcondition(
    resources::resource_wrapper *, uptr_t, const s::postcondition_instance &) const
  { return "Set vertex buffer contents"; }

  const vertex_buffer_contents_var buffer_content{0x200};
}
