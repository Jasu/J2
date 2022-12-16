#pragma once

#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "rendering/vulkan/state/state_variable.hpp"
#include "rendering/vulkan/state/scalar_variable.hpp"

namespace j::rendering::vulkan::state {

  template<class Region, class ResourceRef, typename Value, u32_t Flags = 0>
  class resource_region_variable : public state_variable_definition_base {
    static_assert(sizeof(Value) <= sizeof(uptr_t));
  protected:
    const equals_precondition m_equals;
    const postcondition_definition m_assign{equality_flag};
  public:
    virtual ResourceRef resource_from_region(const Region & region) const = 0;
    virtual uptr_t index_from_region(const Region & region) const = 0;

    virtual Region make_region(ResourceRef resource, uptr_t index) const = 0;

    scalar_condition_helper<Value> operator()(const Region & region) const {
      return {
        resource_from_region(region).wrapper,
        index_from_region(region),
        this,
        &m_equals,
        &m_assign
      };
    }

    bool do_can_transition(const transition_context & context) const override final {
      J_ASSERT_NOT_NULL(context.wrapper);
      return can_transition(
        context,
        make_region(ResourceRef(context.wrapper), context.index),
        *reinterpret_cast<const Value*>(&context.latest_state()),
        *reinterpret_cast<const Value*>(&context.to.data));
    }

    condition_group * do_transition(transition_context & context) const override final {
      J_ASSERT_NOT_NULL(context.wrapper);
      return transition(
        context,
        make_region(ResourceRef(context.wrapper), context.index),
        *reinterpret_cast<const Value*>(&context.latest_state()),
        *reinterpret_cast<const Value*>(&context.to.data));
    }

    virtual bool can_transition(const transition_context &, Region, Value, Value) const {
      return false;
    }

    virtual condition_group * transition(transition_context &, Region, Value, Value) const {
      J_THROW("No automatic transitions.");
    }

    consteval resource_region_variable() noexcept { }

  protected:
    void initialize_state(
      resources::resource_wrapper *,
      uptr_t,
      uptr_t & state
    ) const noexcept override {
      *reinterpret_cast<Value *>(&state) = Value{};
    }
  };
}
