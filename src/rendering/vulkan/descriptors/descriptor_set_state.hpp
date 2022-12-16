#pragma once

#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"
#include "rendering/vulkan/images/image_resource.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"
#include "rendering/vulkan/state/persistent_array_variable.hpp"

namespace j::rendering::vulkan::descriptors {
  struct binding_state {
    uptr_t m_type:1;
    uptr_t m_ptr:63;

    /// Construct an empty binding state.
    constexpr binding_state() noexcept : m_type(0), m_ptr(0) { }

    /// Construct a binding state binding a uniform buffer.
    binding_state(const uniform_buffer_allocation & bound_uniform) noexcept
      : m_type(0),
        m_ptr((reinterpret_cast<uptr_t>(bound_uniform.buffer.wrapper) << 15ULL)
              | bound_uniform.allocation_index)
    {
      J_ASSERT_NOT_NULL(bound_uniform);
    }

    /// Construct a binding state binding a dynamic uniform buffer.
    binding_state(const uniform_buffer_ref & bound_uniform_buffer) noexcept
      : m_type(0),
        m_ptr((reinterpret_cast<uptr_t>(bound_uniform_buffer.wrapper) << 15ULL) | 0xFFFFULL)
    {
      J_ASSERT_NOT_NULL(bound_uniform_buffer.wrapper);
    }

    /// Construct a binding state binding an image.
    binding_state(const images::image_ref & bound_image) noexcept
      : m_type(1),
        m_ptr(reinterpret_cast<uptr_t>(bound_image.wrapper))
    {
      J_ASSERT_NOT_NULL(bound_image);
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return !m_ptr && m_type == 0;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return !m_ptr && m_type == 0;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !m_ptr && m_type == 0;
    }

    J_INLINE_GETTER bool is_uniform() const noexcept {
      return m_ptr && m_type == 0 && !(m_ptr & 1);
    }

    J_INLINE_GETTER bool is_dynamic_uniform() const noexcept {
      return m_ptr && m_type == 0 && (m_ptr & 1);
    }

    J_INLINE_GETTER bool is_image() const noexcept {
      return m_ptr && m_type == 1;
    }

    bool operator==(const binding_state & rhs) const noexcept;

    images::image_ref as_image() const;
    uniform_buffer_ref as_uniform_buffer() const;
    u16_t get_uniform_buffer_allocation_index() const;
  };

  class keyed_binding_state {
  public:
    constexpr keyed_binding_state() noexcept = default;

    keyed_binding_state(u32_t binding, binding_state state) noexcept
      : m_state(state), m_binding(binding)
    {
      J_ASSERT(binding < 16, "Binding index out of range");
    }

    binding_state state() const noexcept {
      return m_state;
    }

    u32_t binding() const noexcept {
      return m_binding;
    }
  private:
    binding_state m_state;
    u32_t m_binding = 0;
  };

  struct descriptor_set_state {
    /// \note The number of bindings is arbitrarily selected to be 16. Some drivers don't
    /// place an upper limit on this, e.g. nVidia supports 4294970000 bindings.
    binding_state bindings[16];
  };

  class descriptor_bindings_variable final
    : public state::persistent_array_variable<
        descriptor_set_ref,
        &descriptor_set_state::bindings
      >
  {
  public:
    using state::persistent_array_variable<descriptor_set_ref, &descriptor_set_state::bindings>::persistent_array_variable;

    strings::string describe_precondition(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const state::precondition_instance & c
    ) const override;

    strings::string describe_postcondition(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const state::postcondition_instance & c
    ) const override;
  protected:
    state::condition_group * transition(
      state::transition_context &,
      descriptor_set_ref ref,
      u32_t index,
      binding_state from,
      binding_state to
    ) const override;

    bool can_transition(
      const state::transition_context &,
      u32_t index,
      binding_state from,
      binding_state to
    ) const override;
  };

  extern const descriptor_bindings_variable descriptor_binding;
}
