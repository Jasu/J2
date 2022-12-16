#include "rendering/vulkan/descriptors/descriptor_set_state.hpp"
#include "rendering/vulkan/operations/update_descriptor_sets_operation.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::descriptors {
  static_assert(sizeof(binding_state) == sizeof(uptr_t));

  bool binding_state::operator==(const binding_state & rhs) const noexcept {
    return m_ptr == rhs.m_ptr && m_type == rhs.m_type;
  }

  images::image_ref binding_state::as_image() const {
    J_ASSERT(is_image(), "Not an image.");
    return images::image_ref(reinterpret_cast<resources::resource_wrapper *>(m_ptr));
  }

  uniform_buffer_ref binding_state::as_uniform_buffer() const {
    J_ASSERT(is_uniform() || is_dynamic_uniform(), "Not a uniform buffer.");
    return uniform_buffer_ref(reinterpret_cast<resources::resource_wrapper *>((m_ptr >> 15ULL) & ~(1ULL)));
  }

  u16_t binding_state::get_uniform_buffer_allocation_index() const {
    J_ASSERT(is_uniform(), "Not a uniform buffer.");
    return m_ptr & ((1 << 15ULL) - 1ULL);
  }

  state::condition_group * descriptor_bindings_variable::transition(
    state::transition_context & context,
    descriptor_set_ref ref,
    u32_t index,
    binding_state,
    binding_state to
  ) const {
    J_ASSERT_NOT_NULL(ref);
    J_ASSERT(!to.empty(), "Target binding cannot be empty.");
    return ::new operations::update_descriptor_sets_operation(
      context.context,
      static_cast<descriptor_set_ref &&>(ref),
      index,
      to
    );
  }

  bool descriptor_bindings_variable::can_transition(
    const state::transition_context &, u32_t, binding_state, binding_state to) const
  { return !to.empty(); }

  strings::string descriptor_bindings_variable::describe_precondition(
    resources::resource_wrapper *, uptr_t, const state::precondition_instance & c) const
  {
    if (&c.definition() == &m_equals) {
      auto b = reinterpret_cast<const binding_state*>(&c.data);
      if (b->is_uniform()) {
        return "Uniform is bound";
      } else if (b->is_image()) {
        return "Image is bound";
      } else if (b->is_dynamic_uniform()) {
        return "Dynamic uniform is bound";
      } else if (b->empty()) {
        return "Empty binding";
      } else {
        return "Invalid descriptor bind precondition";
      }
    } else {
      return "Unknown descriptor bind precondition";
    }
  }

  strings::string descriptor_bindings_variable::describe_postcondition(
    resources::resource_wrapper *, uptr_t, const state::postcondition_instance & c) const
  {
    if (&c.definition() == &m_assign) {
      auto b = reinterpret_cast<const binding_state*>(&c.data);
      if (b->is_uniform()) {
        return "Bind uniform";
      } else if (b->is_dynamic_uniform()) {
        return "Bind dynamic uniform";
      } else if (b->is_image()) {
        return "Bind image";
      } else if (b->empty()) {
        return "Unbind";
      } else {
        return "Invalid descriptor bind postcondition";
      }
    } else {
      return "Unknown descriptor bind postcondition";
    }
  }

  const descriptor_bindings_variable descriptor_binding(0);
}
