#include "rendering/vulkan/state/node_insertion_context.hpp"

#include "rendering/vulkan/state/conditions.hpp"
#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/condition_group.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "rendering/vulkan/state/state_tracker.hpp"

namespace j::rendering::vulkan::state {
  void node_insertion_context::add_condition(const condition_initializer & c) {
    if (c.is_precondition()) {
      tracker.dag->add_precondition(static_cast<const precondition_initializer &>(c));
    } else {
      tracker.dag->add_postcondition(static_cast<const postcondition_initializer &>(c));
    }
  }

  void node_insertion_context::begin_add_node() noexcept {
    start_index = tracker.dag->conditions.size();
  }

  void node_insertion_context::add_precondition(const precondition_initializer & c) {
    tracker.dag->add_precondition(c);
  }

  void node_insertion_context::add_postcondition(const postcondition_initializer & c) {
    tracker.dag->add_postcondition(c);
  }

  void node_insertion_context::move_condition(condition_instance && c) {
    tracker.dag->move_condition(static_cast<condition_instance &&>(c));
  }

  void node_insertion_context::move_condition(condition_group & g, u32_t i) {
    J_ASSERT(g.size() > i, "Out of bounds");
    J_ASSERT(g.conditions_index + i < tracker.dag->conditions.size(), "Out of bounds");
    tracker.dag->move_condition(
      static_cast<condition_instance &&>(
        tracker.dag->conditions.at(g.conditions_index + i)));
  }

  void node_insertion_context::finish_add_node(condition_group * g) noexcept {
    tracker.dag->finish_add_node(g, start_index);
  }
}
