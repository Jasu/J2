#pragma once

#include "hzd/types.hpp"
#include "rendering/vulkan/state/scalar_variable.hpp"
#include "rendering/vulkan/resources/resource_wrapper.hpp"

namespace j::rendering::vulkan::state {
  template<typename ResourceRef, auto MemberPtr>
  class persistent_array_variable;

  template<typename ResourceRef, sz_t Sz, class State, typename Variable, Variable (State::*MemberPtr)[Sz]>
  class persistent_array_variable<ResourceRef, MemberPtr> : public state_variable_definition_base {
  protected:
    const uptr_t m_start_index = 0;
    const equals_precondition m_equals;
    const postcondition_definition m_assign{equality_flag};
  public:
    explicit consteval persistent_array_variable(uptr_t start_index = 0) noexcept
      : m_start_index(start_index)
    {
    }

    void initialize_state(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      uptr_t & state
    ) const noexcept override final {
      index -= m_start_index;
      J_ASSERT(index < Sz, "Out of bounds.");
      J_ASSERT_NOT_NULL(wrapper);
      wrapper->assert_not_empty();
      reinterpret_cast<Variable &>(state) = ((wrapper->resource_state<State>()).*MemberPtr)[index];
    }

    void commit(state_context & context) const override final {
      uptr_t index = context.index - m_start_index;
      J_ASSERT(index < Sz, "Out of bounds.");
      J_ASSERT_NOT_NULL(context.wrapper, context.last_postcondition);
      context.wrapper->assert_not_empty();
      ((context.wrapper->resource_state<State>()).*MemberPtr)[index]
        = reinterpret_cast<const Variable&>(context.last_postcondition->data);
    }

    using condition_data_t = typename scalar_condition_helper<Variable>::condition_data_t;

    scalar_condition_helper<Variable> operator()(const ResourceRef & resource, sz_t index) const {
      resource.assert_not_empty();
      J_ASSERT(index < Sz, "Out of bounds");
      return {
        resource.wrapper,
        m_start_index + index,
        this,
        &m_equals,
        &m_assign
      };
    }
  protected:
    bool do_can_transition(const transition_context & context) const override final
    {
      return can_transition(
        context,
        context.index - m_start_index,
        scalar_condition_helper<Variable>::decode_condition_data(context.latest_state()),
        scalar_condition_helper<Variable>::decode_condition_data(context.to.data));
    }

    condition_group * do_transition(transition_context & context) const override final
    {
      J_ASSERT_NOT_NULL(context.wrapper);
      return transition(
        context,
        ResourceRef(context.wrapper),
        context.index - m_start_index,
        scalar_condition_helper<Variable>::decode_condition_data(context.latest_state()),
        scalar_condition_helper<Variable>::decode_condition_data(context.to.data));
    }

    virtual bool can_transition(const transition_context &, u32_t, condition_data_t, condition_data_t) const {
      return false;
    }

    virtual condition_group * transition(transition_context &, ResourceRef, u32_t, condition_data_t, condition_data_t) const {
      J_THROW("No automatic transitions.");
    }
  };
}
