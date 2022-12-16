#include "rendering/vulkan/state/state_variable.hpp"

#include "rendering/vulkan/state/state_tracker.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "rendering/vulkan/state/state_variable_iterator.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::state {
  state_variable::state_variable(u32_t var_index,
                                 state_variable_key key,
                                 const state_variable_definition_base * definition) noexcept
    : // The masks signify what the index points to - they seem reversed because
      // the head points to tail and the tail points to head.
      head(var_index | variable_tail_mask_v),
      tail(var_index | variable_head_mask_v),
      variable_index(var_index),
      key(key),
      definition(definition)
  {
    // These two asserts are here to check the assert macros themselves.
    J_ASSERT_STATE_IDX_POINTS_TO_VAR_TAIL(head);
    J_ASSERT_STATE_IDX_POINTS_TO_VAR_HEAD(tail);
    J_ASSERT_NOT_NULL(definition);
    definition->initialize_state(key.wrapper, key.index, data);
  }

  void state_variable::commit(state_variable_dag & dag) {
    state_variable_iterator first_postcondition(dag, variable_index),
                            last_postcondition;
    first_postcondition.to_next_postcondition();
    if (first_postcondition.is_postcondition()) {
      last_postcondition = state_variable_iterator(dag, variable_index, true);
      last_postcondition.to_previous_postcondition();
      J_ASSERT(last_postcondition.is_postcondition(), "Must be postcondition");
      state_context c {
        key.index,
        key.wrapper,
        data,
        &first_postcondition->as_postcondition(),
        &last_postcondition->as_postcondition()
      };
      definition->commit(c);
    }
  }


  bool state_variable::can_transition(node_insertion_context & context,
                                      postcondition_instance * last_postcondition,
                                      const precondition_instance & to) const
  {
    J_ASSERT(!last_postcondition || last_postcondition->is_postcondition(),
             "Postcondition mismatch");
    J_ASSERT_NOT_NULL(definition);
    state_variable_iterator first_postcondition(*context.tracker.dag, variable_index);
    J_ASSERT(first_postcondition.is_beginning(), "Iterator fail");
    if (last_postcondition) {
      first_postcondition.to_next_postcondition();
      // If the last postcondition is attached to the graph, at least the same condition
      // should be found as the first postcondition. But if it's only just about to be added,
      // it will not exist, giving the seemingly paradoxical result that the last postcondition
      // exists but the first does not.
      J_ASSERT(first_postcondition.is_postcondition()
               || (first_postcondition.is_end() && last_postcondition->is_detached())
               , "Condition type does not match.");
    }
    const transition_context ctx{
      {
        key.index,
        key.wrapper,
        data,
        first_postcondition.is_postcondition()
          ? &first_postcondition->as_postcondition() : nullptr,
        last_postcondition,
      },
      to,
      const_cast<node_insertion_context &>(context),
    };
    return definition->do_can_transition(ctx);
  }

  condition_group * state_variable::create_transition(node_insertion_context & context,
                                                      postcondition_instance * last_postcondition,
                                                      const precondition_instance & to) const
  {
    J_ASSERT_NOT_NULL(definition);
    state_variable_iterator first_postcondition(*context.tracker.dag, variable_index);
    J_ASSERT(first_postcondition.is_beginning(), "Iterator fail");
    if (last_postcondition) {
      first_postcondition.to_next_postcondition();
      J_ASSERT(first_postcondition.is_postcondition(), "Condition type does not match.");
    }
    transition_context ctx{
      {
        key.index,
        key.wrapper,
        data,
        last_postcondition ? &first_postcondition->as_postcondition()
                           : nullptr,
        last_postcondition,
      },
      to,
      context,
    };
    return definition->do_transition(ctx);
  }

  strings::string state_variable::format_condition(const condition_instance & cond) const {
    return cond.is_precondition()
      ? definition->describe_precondition(key.wrapper, key.index, cond.as_precondition())
      : definition->describe_postcondition(key.wrapper, key.index, cond.as_postcondition());
  }
}
