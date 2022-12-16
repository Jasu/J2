#include "rendering/vulkan/descriptors/descriptor_binding_state.hpp"

#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/state/scalar_variable.hpp"
#include "rendering/vulkan/command_buffers/bind_descriptor_sets_command.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"
#include "strings/format.hpp"

namespace j::rendering::vulkan::descriptors {
  namespace s = state;
  namespace {
    const s::equals_precondition g_binding_equals;
    const s::postcondition_definition g_binding_bind;
  }

  descriptor_set_binding::descriptor_set_binding(uptr_t value) noexcept
    : m_value(value)
  { }

  descriptor_set_binding::descriptor_set_binding(const descriptor_set_ref & set,
                           const uniform_buffer_allocation & dynamic_uniform)
    : descriptor_set_binding(set, dynamic_uniform.offset)
  { }

  descriptor_set_binding::descriptor_set_binding(const descriptor_set_ref & set, u32_t dynamic_binding_offset)
    : descriptor_set_binding::descriptor_set_binding(set)
  {
    J_ASSERT(!(dynamic_binding_offset & ((1 << 5) - 1)), "Dynamic offset not aligned");
    m_value |= ((uptr_t)dynamic_binding_offset << (48ULL - 2ULL - 5ULL));
  }

  descriptor_set_ref descriptor_set_binding::descriptor_set() const noexcept {
    return descriptor_set_ref(reinterpret_cast<resources::resource_wrapper*>((m_value & ((1ULL << (48ULL - 2ULL)) - 1ULL)) << 2ULL));
  }

  u16_t descriptor_set_binding::dynamic_binding_offset() const noexcept {
    return m_value >> (48ULL - 2ULL) << 5ULL;
  }

  s::precondition_initializer descriptor_set_binding_assigner::operator==(descriptor_set_binding value) const noexcept {
    J_ASSERT(m_set_index < 16, "Descriptor set index out of range.");
    return s::precondition_initializer(
      nullptr,
      m_set_index + (uptr_t)command_buffers::command_buffer_state::descriptor_sets_start,
      &g_binding_equals,
      &descriptor_sets,
      (uptr_t)value);
  }

  s::postcondition_initializer descriptor_set_binding_assigner::operator=(descriptor_set_binding value) const noexcept {
    J_ASSERT(m_set_index < 16, "Descriptor set index out of range.");
    return s::postcondition_initializer(
      nullptr,
      m_set_index + (uptr_t)command_buffers::command_buffer_state::descriptor_sets_start,
      &g_binding_bind,
      &descriptor_sets,
      (uptr_t)value);
  }

  s::postcondition_initializer descriptor_set_binding_assigner::clear() const noexcept {
    J_ASSERT(m_set_index < 16, "Descriptor set index out of range.");
    return s::postcondition_initializer(
      nullptr,
      m_set_index + (uptr_t)command_buffers::command_buffer_state::descriptor_sets_start,
      &g_binding_bind,
      &descriptor_sets,
      0);
  }

  strings::string descriptor_set_binding_state_variable::describe_precondition(
    resources::resource_wrapper *,
    uptr_t index,
    [[maybe_unused]] const state::precondition_instance & c
  ) const {
    J_ASSERT(&c.definition() == &g_binding_equals, "Unknown descriptor precondition.");
    index -= (uptr_t)command_buffers::command_buffer_state::descriptor_sets_start;
    return strings::format("Descriptor set {} must be bound", index);
  }

  strings::string descriptor_set_binding_state_variable::describe_postcondition(
    resources::resource_wrapper *,
    uptr_t index,
    const state::postcondition_instance & c
  ) const {
    J_ASSERT(&c.definition() == &g_binding_bind, "Unknown descriptor postcondition.");
    index -= (uptr_t)command_buffers::command_buffer_state::descriptor_sets_start;
    return strings::format(c.data ? "Bind descriptor set {}" : "Unbind descriptor set {}", index);
  }

  bool descriptor_set_binding_state_variable::do_can_transition(
    const state::transition_context & context) const {
    return context.to.data;
  }

  state::condition_group * descriptor_set_binding_state_variable::do_transition(state::transition_context & context) const {
    descriptor_set_binding data{context.to.data};
    descriptor_set_ref set{data.descriptor_set()};
    J_ASSERT_NOT_NULL(set);
    return ::new command_buffers::bind_descriptor_sets_command(
      context.context,
      context.index - (uptr_t)command_buffers::command_buffer_state::descriptor_sets_start,
      set,
      data.dynamic_binding_offset());
  }

  const descriptor_set_binding_state_variable descriptor_sets;
}
