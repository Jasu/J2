#pragma once

#include "rendering/vulkan/state/common.hpp"

namespace j::rendering::vulkan::state {
  struct state_variable;
  class condition_group;
  class postcondition_definition;
  class postcondition_instance;
  class precondition_definition;
  class precondition_instance;
  class state_variable_definition_base;

  struct condition_initializer {
    state_variable_key key;
    uptr_t definition_uintptr;
    const state_variable_definition_base * variable_definition;
    uptr_t data;

    bool is_precondition() const noexcept {
      return !(definition_uintptr & 1);
    }
  };

  struct postcondition_initializer : condition_initializer {
    postcondition_initializer(resources::resource_wrapper * wrapper, uptr_t index,
                              const postcondition_definition * J_NOT_NULL definition,
                              const state_variable_definition_base * J_NOT_NULL variable_definition,
                              uptr_t data) noexcept
      : condition_initializer{
        .key{index, wrapper},
        .definition_uintptr = reinterpret_cast<uptr_t>(definition) | 3ULL,
        .variable_definition = variable_definition,
        .data = data}
    { }
  };

  struct precondition_initializer : condition_initializer {
    precondition_initializer(resources::resource_wrapper * wrapper, uptr_t index,
                          const precondition_definition * J_NOT_NULL definition,
                          const state_variable_definition_base * J_NOT_NULL variable_definition,
                          uptr_t data) noexcept
      : condition_initializer{
        .key{index, wrapper},
        .definition_uintptr = reinterpret_cast<uptr_t>(definition) | 2ULL,
        .variable_definition = variable_definition,
        .data = data}
    { }
  };

  enum class precondition_change {
    unsatisfied,
    satisfied,
    unchanged,
  };

  class condition_instance {
  public:
    u32_t next = U16_MAX;
    u32_t previous = U16_MAX;
    uptr_t m_definition_uintptr = 0UL;
    uptr_t data = 0UL;
    union {
      condition_group * group = nullptr;
      u32_t state_variable_index;
    };

    J_A(AI,ND) constexpr condition_instance() noexcept = default;

    J_INLINE_GETTER bool is_postcondition() const noexcept
    { return m_definition_uintptr & 1; }

    J_INLINE_GETTER bool is_precondition() const noexcept
    { return !(m_definition_uintptr & 1); }

    J_INLINE_GETTER bool is_initialized() const noexcept {
      return !(m_definition_uintptr & 2);
    }

    J_A(AI,ND) condition_instance(const condition_initializer & init, u32_t state_variable_index) noexcept
      : m_definition_uintptr(init.definition_uintptr),
        data(init.data),
        state_variable_index(state_variable_index)
    { }

    /// Check whether the condition has not yet been added to the graph.
    bool is_detached() const noexcept { return next == U16_MAX; }

    void initialize(condition_group * J_NOT_NULL group) noexcept {
      J_ASSUME(m_definition_uintptr & 2);
      m_definition_uintptr -= 2;
      this->group = group;
    }

    J_INLINE_GETTER precondition_instance & as_precondition() noexcept {
      J_ASSERT(!is_postcondition());
      return *reinterpret_cast<precondition_instance*>(this);
    }

    J_INLINE_GETTER postcondition_instance & as_postcondition() noexcept {
      J_ASSERT(is_postcondition());
      return *reinterpret_cast<postcondition_instance*>(this);
    }

    J_INLINE_GETTER const precondition_instance & as_precondition() const noexcept {
      J_ASSERT(!is_postcondition());
      return *reinterpret_cast<const precondition_instance*>(this);
    }

    J_INLINE_GETTER const postcondition_instance & as_postcondition() const noexcept {
      J_ASSERT(is_postcondition());
      return *reinterpret_cast<const postcondition_instance*>(this);
    }

    friend struct state_variable;
    friend class state_tracker;
  };

  class precondition_instance final : public condition_instance {
  public:
    using condition_instance::condition_instance;

    J_INLINE_GETTER const precondition_definition & definition() const noexcept {
      return *reinterpret_cast<const precondition_definition *>(m_definition_uintptr);
    }

    inline bool is_initially_satisfied(const state_variable & state) const;
    inline precondition_change get_change(const postcondition_instance & postcondition) const;
  };

  class postcondition_instance final : public condition_instance {
  public:
    using condition_instance::condition_instance;

    J_INLINE_GETTER const postcondition_definition & definition() const noexcept {
      return *reinterpret_cast<const postcondition_definition *>(m_definition_uintptr - 1ULL);
    }
  };

}
