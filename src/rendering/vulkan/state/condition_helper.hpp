#pragma once

#include "rendering/vulkan/state/condition_definition.hpp"

namespace j::rendering::vulkan::resources {
  template<typename, typename, typename> class resource_ref;
}
namespace j::rendering::vulkan::state {
  class condition_helper_base {
    const uptr_t m_index;
    resources::resource_wrapper * const m_wrapper;
    const state_variable_definition_base * const m_state_variable_definition;
    const precondition_definition * m_equals_precondition;
    const postcondition_definition * m_assign_postcondition;
  public:
    J_ALWAYS_INLINE constexpr condition_helper_base(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const state_variable_definition_base * state_variable_definition,
      const precondition_definition * equals_precondition,
      const postcondition_definition * assign_postcondition
    ) noexcept
      : m_index(index),
        m_wrapper(wrapper),
        m_state_variable_definition(state_variable_definition),
        m_equals_precondition(equals_precondition),
        m_assign_postcondition(assign_postcondition)
    { }

    J_ALWAYS_INLINE precondition_initializer make_equals(uptr_t value) const noexcept {
      return precondition_initializer{
        m_wrapper,
        m_index,
        m_equals_precondition,
        m_state_variable_definition,
        value,
      };
    }

    J_ALWAYS_INLINE postcondition_initializer make_assign(uptr_t value) const noexcept {
      return postcondition_initializer{
        m_wrapper,
        m_index,
        m_assign_postcondition,
        m_state_variable_definition,
        value,
      };
    }
  };

  template<typename Type>
  class scalar_condition_helper : public condition_helper_base {
  public:
    using condition_helper_base::condition_helper_base;
    J_ALWAYS_INLINE precondition_initializer operator==(Type t) const noexcept {
      uptr_t value = 0;
      *reinterpret_cast<Type*>(&value) = t;
      return make_equals(value);
    }

    J_ALWAYS_INLINE postcondition_initializer operator=(Type t) const noexcept {
      uptr_t value = 0;
      *reinterpret_cast<Type*>(&value) = t;
      return make_assign(value);
    }

    using condition_data_t J_NO_DEBUG_TYPE = Type;
    J_ALWAYS_INLINE static condition_data_t decode_condition_data(uptr_t data) noexcept {
      return *reinterpret_cast<condition_data_t*>(&data);
    }
  };

  template<typename Res, typename Desc, typename Def>
  class scalar_condition_helper<resources::resource_ref<Res, Desc, Def>> : public condition_helper_base {
  public:
    using condition_helper_base::condition_helper_base;
    J_ALWAYS_INLINE precondition_initializer operator==(const resources::resource_ref<Res, Desc, Def> & ref) const noexcept {
      return make_equals(reinterpret_cast<uptr_t>(ref.wrapper));
    }

    J_ALWAYS_INLINE postcondition_initializer operator=(const resources::resource_ref<Res, Desc, Def> & ref) const noexcept {
      return make_assign(reinterpret_cast<uptr_t>(ref.wrapper));
    }

    using condition_data_t J_NO_DEBUG_TYPE = resources::resource_wrapper *;
    J_ALWAYS_INLINE static condition_data_t decode_condition_data(uptr_t data) noexcept {
      return reinterpret_cast<resources::resource_wrapper*>(data);
    }
  };
}
