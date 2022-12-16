#pragma once

#include "rendering/vulkan/push_constants/special_push_constants.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/state/conditions.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
    class push_constant_state_assigner {
    public:
      state::precondition_initializer operator==(special_push_constant value) const noexcept;
      state::postcondition_initializer operator=(special_push_constant value) const noexcept;
      state::precondition_initializer operator==(const void * value) const noexcept;
      state::postcondition_initializer operator=(const void * value) const noexcept;
      state::postcondition_initializer clear() const noexcept;


      push_constant_state_assigner & operator=(const push_constant_state_assigner &) = delete;
      push_constant_state_assigner(const push_constant_state_assigner &) = delete;
      push_constant_state_assigner & operator=(push_constant_state_assigner &&) = delete;
      push_constant_state_assigner(push_constant_state_assigner &&) = delete;
    private:
      u8_t m_value;

      explicit push_constant_state_assigner(u8_t value) noexcept
        : m_value(value) { }
      friend class push_constant_state_variable;
    };

    class push_constant_state_variable final : public state::state_variable_definition_base {
    public:
      J_INLINE_GETTER push_constant_state_assigner operator[](u8_t push_constant_index) const noexcept {
        return push_constant_state_assigner(push_constant_index);
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
      void initialize_state(resources::resource_wrapper *, uptr_t, uptr_t & state) const noexcept override;
      bool do_can_transition(const state::transition_context & context) const override;
      state::condition_group * do_transition(state::transition_context & context) const override;
    };

    extern const push_constant_state_variable push_constant;
  }
}
