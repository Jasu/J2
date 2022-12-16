#include "rendering/vulkan/geometry/vertex_buffer_suballocation_state.hpp"
#include "rendering/vulkan/geometry/vertex_buffer.hpp"
#include "rendering/vulkan/operations/copy_to_vertex_buffer_operation.hpp"
#include "rendering/vertex_data/vertex_data_source_manager.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::geometry {
  namespace s = state;
  vertex_buffer_ref vertex_buffer_suballocation_var::resource_from_region(const vertex_buffer_suballocation & src) const {
    J_ASSERT_NOT_NULL(src);
    return src.buffer();
  }

  uptr_t vertex_buffer_suballocation_var::index_from_region(const vertex_buffer_suballocation & src) const {
    J_ASSERT_NOT_NULL(src);
    return (uptr_t)src.inner();
  }

  vertex_buffer_suballocation vertex_buffer_suballocation_var::make_region([[maybe_unused]]vertex_buffer_ref buf,
                                                                           uptr_t index) const {
    J_ASSERT_NOT_NULL(buf, index);
    return vertex_buffer_suballocation((buffers::generic_suballocation *)index);
  }

  strings::string vertex_buffer_suballocation_var::describe_precondition(
    resources::resource_wrapper *, uptr_t, const s::precondition_instance &) const
  { return "Vertex allocation must contain"; }

  strings::string vertex_buffer_suballocation_var::describe_postcondition(
    resources::resource_wrapper *, uptr_t, const s::postcondition_instance &) const
  { return "Vertex allocation contains"; }

  bool vertex_buffer_suballocation_var::can_transition(
    const s::transition_context &,
    vertex_buffer_suballocation source,
    vertex_data::vertex_data_source_key from,
    vertex_data::vertex_data_source_key to
    ) const
  {
    return from != to && to && source;
  }

  s::condition_group * vertex_buffer_suballocation_var::transition(
    s::transition_context & context,
    vertex_buffer_suballocation source,
    [[maybe_unused]] vertex_data::vertex_data_source_key from,
    vertex_data::vertex_data_source_key to
  ) const {
    J_ASSERT(can_transition(context, source, from, to), "Cannot transition vertex suballocation.");
    auto info = vertex_data::vertex_data_source_manager::instance.get_info(to);
    J_ASSERT(info.stride_bytes() <= source.size(), "Allocation does not fit.");
    return ::new operations::copy_to_vertex_buffer_operation(
      context.context,
      to,
      vertex_data::vertex_buffer_view(info, source.target()),
      (*this)(source) = to
    );
  }

  void vertex_buffer_suballocation_var::initialize_state(
    [[maybe_unused]] resources::resource_wrapper * buffer,
    uptr_t index,
    uptr_t & J_RESTRICT state) const noexcept
  {
    J_ASSERT_NOT_NULL(buffer, index);
    const vertex_buffer_suballocation alloc((buffers::generic_suballocation *)index);
    state = alloc.userdata();
  }

  void vertex_buffer_suballocation_var::commit(state::state_context & context) const {
    J_ASSERT_NOT_NULL(context.wrapper, context.index);
    vertex_buffer_suballocation alloc((buffers::generic_suballocation *)context.index);
    alloc.userdata() = context.latest_state();
  }

  const vertex_buffer_suballocation_var vertex_suballocation_contents;
}
