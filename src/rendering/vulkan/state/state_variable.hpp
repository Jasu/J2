#pragma once

#include "rendering/vulkan/state/conditions.hpp"
#include "rendering/vulkan/state/node_insertion_context.hpp"
#include "rendering/vulkan/state/common.hpp"

namespace j::rendering::vulkan::state {
  class condition_group;
  class precondition_instance;
  class postcondition_instance;
  class state_variable_dag;
  class state_variable_definition_base;

  struct state_variable final {
    u32_t head = 0U;
    u32_t tail = 0U;
    u32_t variable_index = 0U;
    state_variable_key key;

    /// The initial state of the state variable.
    uptr_t data = 0;
    const state_variable_definition_base * definition = nullptr;

    state_variable(u32_t var_index,
                   state_variable_key key,
                   const state_variable_definition_base * definition) noexcept;

    void commit(state_variable_dag & dag);

    strings::string format_condition(const condition_instance & cond) const;

    bool can_transition(node_insertion_context & context,
                        postcondition_instance * last_postcondition,
                        const precondition_instance & to) const;

    condition_group * create_transition(node_insertion_context & context,
                                        postcondition_instance * last_postcondition,
                                        const precondition_instance & to) const;
  };

  J_INLINE_GETTER bool operator==(const state_variable & lhs, const state_variable_key & rhs) noexcept {
    return lhs.key == rhs;
  }

  J_INLINE_GETTER bool operator==(const state_variable_key & lhs, const state_variable & rhs) noexcept {
    return lhs == rhs.key;
  }
}
