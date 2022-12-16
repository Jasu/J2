#pragma once

#include "rendering/vulkan/state/state_variable_definition.hpp"

#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    class uniform_buffer_allocation;
  }
}

namespace j::rendering::vulkan::descriptors {
  class descriptor_set_binding {
  public:
    descriptor_set_binding() noexcept = default;
    explicit descriptor_set_binding(uptr_t value) noexcept;
    descriptor_set_binding(const descriptor_set_ref & set)
      : m_value(reinterpret_cast<uptr_t>(set.wrapper))
    {
      J_ASSERT_NOT_NULL(set);
      J_ASSERT(!(m_value & 3), "Unaligned wrapper");
      m_value >>= 2;
    }
    descriptor_set_binding(const descriptor_set_ref & set,
                           const uniform_buffer_allocation & dynamic_uniform);
    descriptor_set_binding(const descriptor_set_ref & set, u32_t dynamic_binding_offset);

    descriptor_set_ref descriptor_set() const noexcept;
    u16_t dynamic_binding_offset() const noexcept;
    explicit operator uptr_t() const noexcept {
      return m_value;
    }
  private:
    uptr_t m_value;
  };

  class descriptor_set_binding_assigner {
  public:
    state::precondition_initializer operator==(descriptor_set_binding binding) const noexcept;
    state::postcondition_initializer operator=(descriptor_set_binding binding) const noexcept;
    state::postcondition_initializer clear() const noexcept;

    descriptor_set_binding_assigner(const descriptor_set_binding_assigner &) = delete;
  private:
    const u8_t m_set_index;
    explicit constexpr descriptor_set_binding_assigner(u8_t set_index) noexcept : m_set_index(set_index) { }
    friend class descriptor_set_binding_state_variable;
  };

  class descriptor_set_binding_state_variable final : public state::state_variable_definition_base {
  public:
    J_INLINE_GETTER descriptor_set_binding_assigner operator[](u8_t set_index) const noexcept {
      return descriptor_set_binding_assigner(set_index);
    }

    strings::string describe_precondition(
      resources::resource_wrapper * wrapper,
      uptr_t detail,
      const state::precondition_instance & c
    ) const override;

    strings::string describe_postcondition(
      resources::resource_wrapper * wrapper,
      uptr_t detail,
      const state::postcondition_instance & c
    ) const override;
  protected:
    bool do_can_transition(const state::transition_context & context) const override;
    state::condition_group * do_transition(state::transition_context & context) const override;
  };

  extern const descriptor_set_binding_state_variable descriptor_sets;
}
