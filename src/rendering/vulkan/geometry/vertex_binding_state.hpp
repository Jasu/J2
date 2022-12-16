#pragma once

#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"

namespace j::rendering::vulkan::geometry {
  class vertex_binding_assigner final {
  public:
    state::precondition_initializer operator==(const vertex_buffer_ref & buffer) const noexcept;
    state::postcondition_initializer operator=(const vertex_buffer_ref & buffer) const noexcept;

    vertex_binding_assigner(const vertex_binding_assigner &) = delete;
  private:
    const u8_t m_index;
    explicit constexpr vertex_binding_assigner(u8_t index) noexcept : m_index(index) { }
    friend class vertex_binding_state_variable;
  };

  class vertex_binding_state_variable final : public state::state_variable_definition_base {
  public:
    J_INLINE_GETTER constexpr vertex_binding_assigner operator[](u8_t binding_index) const noexcept {
      return vertex_binding_assigner{binding_index};
    }

    strings::string describe_precondition(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const state::precondition_instance & c
    ) const override;

    virtual strings::string describe_postcondition(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const state::postcondition_instance & c
    ) const override;
  protected:
    bool do_can_transition(const state::transition_context & context) const override;
    state::condition_group * do_transition(state::transition_context & context) const override;
  };

  extern const vertex_binding_state_variable vertex_bindings;
}
