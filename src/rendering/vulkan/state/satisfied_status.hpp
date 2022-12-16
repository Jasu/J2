#pragma once

#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/state_iterator.hpp"
#include "rendering/vulkan/state/state_tracker.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "bits/bitops.hpp"
#include "debug/counters.hpp"
#include "exceptions/assert_lite.hpp"

J_DEFINE_COUNTERS("Vulkan State", get_postcondition_change, get_precondition_change, can_transition,
                  can_insert_precondition_after_call, can_insert_postcondition_after_call,
                  equality_checks, equality_successes)

namespace j::rendering::vulkan::state {
  inline bool can_insert_postcondition_after(postcondition_instance & postcondition,
                                             u32_t flags,
                                             state_variable_iterator at,
                                             condition_instance ** failing_condition) {
    J_ASSERT(!at.is_end(), "Cannot insert after end.");
    J_ASSERT_NOT_NULL(failing_condition);
    if (flags & noninvalidating_flag) {
      return true;
    }
    const bool is_equal = (flags & equality_flag) == equality_flag;
    J_COUNT(can_insert_postcondition_after_call);
    if (at.is_beginning()) {
      at.to_next(at_begin);
    } else {
      at.to_next_group(at_condition);
    }
    // TODO: Fix the assumption that postcondition completely resets vailidity
    //       (i.e. don't terminate the check upon meeting the first postcondition.)
    while (!at.is_end() && at->is_precondition()) {
      // J_DEBUG("  Considering to insert {} after {}: {}",
      //         at.var().format_condition(postcondition),
      //         at.group()->name(),
      //         at.var().format_condition(*at));
      J_COUNT(get_postcondition_change);
      if (is_equal && (at->as_precondition().definition().flags & equality_flag) == equality_flag) {
        if (at->data == postcondition.data) {
          return true;
        }
        *failing_condition = &*at;
        return false;
      }
      switch (at->as_precondition().get_change(postcondition)) {
      case precondition_change::unsatisfied: *failing_condition = &*at; return false;
      case precondition_change::satisfied: case precondition_change::unchanged: break;
      }
      at.to_next(at_condition);
    }
    return true;
  }

  inline bool can_insert_precondition_after(precondition_instance & precondition,
                                            u32_t flags,
                                            state_variable_iterator at,
                                            condition_instance ** failing_condition) {
    J_ASSERT_NOT_NULL(failing_condition);
    J_COUNT(can_insert_precondition_after_call);
    const bool is_equal = (flags & equality_flag) == equality_flag;
    const bool is_modal = flags & modal_flag;
    for (;;) {
      while (at.is_precondition()) {
        if (is_modal) {
          J_COUNT(equality_checks);
          auto & cond = *at;
          // if (J_LIKELY(&cond.as_precondition().definition() == &precondition.definition())) {
            if (cond.data == precondition.data) {
              J_COUNT(equality_successes);
              return true;
            } else if (is_equal) {
              at.to_previous_postcondition(at_condition);
              *failing_condition = at.is_beginning() ? nullptr : &*at;
              return false;
            }
          // }
        }
        at.to_previous(at_condition);
      }
      if (!at.is_at_condition()) {
        break;
      }
      if (is_equal && (at->as_postcondition().definition().flags & equality_flag) == equality_flag) {
        if (at->data == precondition.data) {
          return true;
        }
        *failing_condition = &*at;
        return false;
      }

      // J_DEBUG("  Considering to insert {} after {}: {}",
      //         at.var().format_condition(precondition),
      //         at.group()->name(),
      //         at.var().format_condition(*at));
      J_COUNT(get_precondition_change);
      switch (precondition.get_change(at->as_postcondition())) {
      case precondition_change::satisfied:   return true;
      case precondition_change::unsatisfied: *failing_condition = &*at; return false;
      case precondition_change::unchanged:   break;
      }
      at.to_previous(at_condition);
    }
    return is_equal ? at.var().data == precondition.data : precondition.is_initially_satisfied(at.var());
  }

  /// Bitmask tracking whether conditions are satisfied and whether they are satisfiable.
  class satisfied_status final {
  public:
    constexpr satisfied_status() noexcept = default;

    satisfied_status(node_insertion_context & context,
                     const state_iterator & it,
                     u32_t start_index,
                     const u32_t * flags,
                     u32_t num_preconditions)
    {
      initialize_at(context, it, start_index, flags, num_preconditions);
    }

    void initialize_condition_at(
      node_insertion_context & context,
      state_variable_iterator it,
      condition_instance & cond,
      u32_t flags,
      u32_t i,
      bool is_precondition)
    {
      J_ASSERT(!it.is_end(), "Cannot insert after end.");
      condition_instance * fail = nullptr;
      const bool state = is_precondition
        ? can_insert_precondition_after(cond.as_precondition(), flags, it, &fail)
        : can_insert_postcondition_after(cond.as_postcondition(), flags, it, &fail);
      if (state) {
        satisfied_mask |= (1ULL << i);
      } else {
        J_COUNT(can_transition);
        if (!it.var().can_transition(
              context,
              is_precondition ? static_cast<postcondition_instance*>(fail) : &cond.as_postcondition(),
              is_precondition ? cond.as_precondition() : fail->as_precondition()))
        {
          unsatisfiable_mask |= (1ULL << i);
        }
      }
    }


    void initialize_at(node_insertion_context & context,
                       const state_iterator & it,
                       u32_t start_index,
                       const u32_t * flags,
                       u32_t num_preconditions)
    {
      satisfied_mask = unsatisfiable_mask = 0ULL;
      const state_variable_iterator * const its = it.begin();
      const u32_t num_vars = it.size();
      J_ASSUME(num_vars > 0);
      auto * p = context.tracker.dag->conditions.begin();
      p += start_index;
      for (u32_t i = 0; i < num_vars; ++i, ++p) {
        initialize_condition_at(context, its[i], *p, *flags++, i, i < num_preconditions);
      }
    }

    void update_precondition(node_insertion_context & context,
                             [[maybe_unused]] u32_t flags,
                             precondition_instance & precondition,
                             u8_t index,
                             state_variable_iterator it)
    {
      const u64_t mask = 1ULL << index;
      if (!it.is_postcondition()) { return; }
      J_COUNT(get_precondition_change);
      switch (precondition.get_change(it->as_postcondition())) {
        case precondition_change::satisfied:
          satisfied_mask |= mask;
          unsatisfiable_mask &= ~mask;
          return;
        case precondition_change::unsatisfied:
          satisfied_mask &= ~mask;
          break;
        case precondition_change::unchanged: break;
      }
      if (!(satisfied_mask & mask)) {
        J_COUNT(can_transition);
        if (it.var().can_transition(context, &it->as_postcondition(), precondition)) {
          unsatisfiable_mask &= ~mask;
        } else {
          unsatisfiable_mask |= mask;
        }
      }
    }

    void update_postcondition(node_insertion_context & context,
                              u32_t flags,
                              postcondition_instance & postcondition,
                              u8_t index,
                              state_variable_iterator it)
    {
      if (it.is_end() || it.next_group(at_condition) == it->group) { return; }
      const u64_t mask = 1ULL << index;
      condition_instance * fail = nullptr;
      if (can_insert_postcondition_after(postcondition, flags, it, &fail)) {
        satisfied_mask |= mask;
        unsatisfiable_mask &= ~mask;
      } else {
        J_ASSERT_NOT_NULL(fail);
        satisfied_mask &= ~mask;
        J_COUNT(can_transition);
        if (it.var().can_transition(context, &postcondition, fail->as_precondition())) {
          unsatisfiable_mask &= ~mask;
        } else {
          unsatisfiable_mask |= mask;
        }
      }
    }

    void update_condition(bool is_precondition,
                          node_insertion_context & context,
                          u32_t flags,
                          condition_instance & cond,
                          u8_t index,
                          state_variable_iterator it)
    {
      if (is_precondition) {
        update_precondition(context, flags, cond.as_precondition(), index, it);
      } else {
        update_postcondition(context, flags, cond.as_postcondition(), index, it);
      }
    }

    bool is_satisfied(u8_t i) const noexcept {
      J_ASSUME(i < 64U);
      return satisfied_mask & (1ULL << i);
    }

    J_INLINE_GETTER u8_t num_satisfied() const noexcept {
      return bits::popcount(satisfied_mask);
    }

    J_INLINE_GETTER bool has_unsatisfiable() const noexcept {
      return unsatisfiable_mask;
    }
  private:
    u64_t satisfied_mask = 0ULL;
    u64_t unsatisfiable_mask = 0ULL;
  };
}
