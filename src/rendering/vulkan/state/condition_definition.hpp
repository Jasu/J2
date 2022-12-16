#pragma once

#include "rendering/vulkan/state/conditions.hpp"

namespace j::rendering::vulkan::state {
  inline constexpr u32_t noninvalidating_flag = 1U;
  inline constexpr u32_t modal_flag = 2U;
  inline constexpr u32_t equality_flag = 6U;

  class condition_definition_base {
  public:
    u32_t flags;

    J_ALWAYS_INLINE consteval condition_definition_base(u32_t flags = 0U) noexcept
      : flags(flags)
    { }

    virtual ~condition_definition_base();

    condition_definition_base(const condition_definition_base &) = delete;
    condition_definition_base operator=(const condition_definition_base &) = delete;
  };

  class precondition_definition : public condition_definition_base {
  public:
    using condition_definition_base::condition_definition_base;

    virtual bool get_initial_state(const state_variable & state, uptr_t precondition_data) const;

    virtual precondition_change get_change(
      const postcondition_instance & postcondition,
      uptr_t precondition_data) const = 0;
  };

  class postcondition_definition : public condition_definition_base {
  public:
    using condition_definition_base::condition_definition_base;
  };

  J_INLINE_GETTER bool precondition_instance::is_initially_satisfied(const state_variable & state) const {
    return reinterpret_cast<const precondition_definition *>(m_definition_uintptr)->get_initial_state(state, data);
  }

  J_INLINE_GETTER precondition_change precondition_instance::get_change(const postcondition_instance & postcondition) const {
    return reinterpret_cast<const precondition_definition *>(m_definition_uintptr)->get_change(postcondition, data);
  }
}
