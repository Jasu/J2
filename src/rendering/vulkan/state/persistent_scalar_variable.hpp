#pragma once

#include "rendering/vulkan/state/scalar_variable.hpp"
#include "rendering/vulkan/resources/resource_wrapper.hpp"

namespace j::rendering::vulkan::state {
  template<typename ResourceRef, auto MemberPtr>
  class persistent_scalar_variable;

  template<typename ResourceRef, class State, typename Variable, Variable State::* MemberPtr>
  class persistent_scalar_variable<ResourceRef, MemberPtr>
    : public scalar_variable_base<ResourceRef, Variable>
  {
  public:
    using scalar_variable_base<ResourceRef, Variable>::scalar_variable_base;
    using scalar_variable_base<ResourceRef, Variable>::operator();

    void initialize_state(
      resources::resource_wrapper * wrapper,
      uptr_t,
      uptr_t & state
    ) const noexcept override final {
      J_ASSERT_NOT_NULL(wrapper);
      wrapper->assert_not_empty();
      *reinterpret_cast<Variable *>(&state) = (wrapper->resource_state<State>()).*MemberPtr;
    }

    void commit(state_context & context) const override final {
      J_ASSERT_NOT_NULL(context.wrapper, context.last_postcondition);
      context.wrapper->assert_not_empty();
      (context.wrapper->resource_state<State>()).*MemberPtr
        = reinterpret_cast<const Variable&>(context.last_postcondition->data);
    }
  };
}
