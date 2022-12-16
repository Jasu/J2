#include "rendering/vulkan/geometry/vertex_binding_state.hpp"
#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "strings/format.hpp"
#include "rendering/vulkan/command_buffers/bind_vertex_buffers_command.hpp"

namespace j::rendering::vulkan::geometry {
  namespace s = state;
  namespace {
    const s::equals_precondition g_binding_equals;
    const s::postcondition_definition g_binding_bind;
  }

  s::precondition_initializer vertex_binding_assigner::operator==(const vertex_buffer_ref & buffer) const noexcept {
    J_ASSERT_NOT_NULL(buffer);
    return s::precondition_initializer(
      nullptr,
      m_index + (uptr_t)command_buffers::command_buffer_state::vertex_bindings_start,
      &g_binding_equals,
      &vertex_bindings,
      (uptr_t)buffer.wrapper);
  }

  s::postcondition_initializer vertex_binding_assigner::operator=(const vertex_buffer_ref & buffer) const noexcept {
    J_ASSERT_NOT_NULL(buffer);
    return s::postcondition_initializer(
      nullptr,
      m_index + (uptr_t)command_buffers::command_buffer_state::vertex_bindings_start,
      &g_binding_bind,
      &vertex_bindings,
      (uptr_t)buffer.wrapper);
  }

  strings::string vertex_binding_state_variable::describe_precondition(
    resources::resource_wrapper *,
    uptr_t index,
    [[maybe_unused]] const s::precondition_instance & c) const
  {
    J_ASSERT(&c.definition() == &g_binding_equals, "Unknown vertex binding precondition.");
    return strings::format(
      "Vertex binding #{} is bound to buffer",
      index - (uptr_t)command_buffers::command_buffer_state::vertex_bindings_start);
  }

  strings::string vertex_binding_state_variable::describe_postcondition(
    resources::resource_wrapper *,
    uptr_t index,
    [[maybe_unused]] const s::postcondition_instance & c) const
  {
    J_ASSERT(&c.definition() == &g_binding_bind, "Unknown vertex binding postcondition.");
    return strings::format(
      "Bind vertex binding #{} to buffer",
      index - (uptr_t)command_buffers::command_buffer_state::vertex_bindings_start);
  }

  bool vertex_binding_state_variable::do_can_transition(const state::transition_context & context) const {
    return context.to.data;
  }

  state::condition_group * vertex_binding_state_variable::do_transition(state::transition_context & context) const {
    resources::resource_wrapper * const wrapper = reinterpret_cast<resources::resource_wrapper *>(context.to.data);
    J_ASSERT_NOT_NULL(wrapper);
    return ::new command_buffers::bind_vertex_buffers_command(
      context.context,
      context.index - (uptr_t)command_buffers::command_buffer_state::vertex_bindings_start,
      vertex_buffer_ref(wrapper),
      0U);
  }

  const vertex_binding_state_variable vertex_bindings;
}
