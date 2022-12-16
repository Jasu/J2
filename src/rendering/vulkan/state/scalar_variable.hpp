#pragma once

#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "rendering/vulkan/state/condition_helper.hpp"

namespace j::rendering::vulkan::state {
  class equals_precondition final : public precondition_definition {
  public:
    consteval equals_precondition() noexcept : precondition_definition(equality_flag)
    { }

    bool get_initial_state(const state_variable & state, uptr_t precondition_data) const override;
    precondition_change get_change(
      const postcondition_instance & postcondition,
      uptr_t precondition_data) const override;
  };

  template<typename ResourceRef, typename Type>
  class scalar_variable_base : public state_variable_definition_base {
    uptr_t m_index;
  protected:
    equals_precondition m_equals;
    postcondition_definition m_assign{equality_flag};
  public:
    template<typename Detail>
    consteval scalar_variable_base(Detail index) noexcept
      : m_index(static_cast<uptr_t>(index))
    {
    }

    bool do_can_transition(const transition_context & context) const override final {
      return can_transition(
        context,
        *reinterpret_cast<const Type*>(&context.latest_state()),
        *reinterpret_cast<const Type*>(&context.to.data));
    }

    condition_group * do_transition(transition_context & context) const override final {
      J_ASSERT_NOT_NULL(context.wrapper);
      return transition(
        context,
        ResourceRef(context.wrapper),
        *reinterpret_cast<const Type*>(&context.latest_state()),
        *reinterpret_cast<const Type*>(&context.to.data));
    }

    virtual bool can_transition(const transition_context &, Type, Type) const {
      return false;
    }

    virtual condition_group * transition(transition_context &, ResourceRef, Type, Type) const {
      J_THROW("No automatic transitions.");
    }

    scalar_condition_helper<Type> operator()(const ResourceRef & resource) const {
      resource.assert_not_empty();
      return scalar_condition_helper<Type>{
        resource.wrapper,
        m_index,
        this,
        &m_equals,
        &m_assign
      };
    }
  };

  template<typename Type>
  class global_scalar_variable
    : public state_variable_definition_base,
      public scalar_condition_helper<Type>
  {
  protected:
    const equals_precondition m_equals;
    const postcondition_definition m_assign{equality_flag};
  public:
    template<typename Detail>
    consteval global_scalar_variable(Detail index) noexcept
      : scalar_condition_helper<Type>(
        nullptr,
        static_cast<uptr_t>(index),
        this,
        &m_equals,
        &m_assign
      )
    {
    }

    using typename scalar_condition_helper<Type>::condition_data_t;
    using scalar_condition_helper<Type>::operator==;
    using scalar_condition_helper<Type>::operator=;

  protected:
    bool do_can_transition(const transition_context & context) const override final
    {
      return can_transition(
        context,
        scalar_condition_helper<Type>::decode_condition_data(context.latest_state()),
        scalar_condition_helper<Type>::decode_condition_data(context.to.data));
    }

    condition_group * do_transition(transition_context & context) const override final
    {
      return transition(
        context,
        scalar_condition_helper<Type>::decode_condition_data(context.latest_state()),
        scalar_condition_helper<Type>::decode_condition_data(context.to.data));
    }

    virtual bool can_transition(const transition_context &, condition_data_t, condition_data_t) const {
      return false;
    }

    virtual condition_group * transition(transition_context &, condition_data_t, condition_data_t) const {
      J_THROW("No automatic transitions.");
    }
  };
}
