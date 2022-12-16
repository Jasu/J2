#include "rendering/vulkan/state/state_tracker.hpp"

#include "containers/obstack.hpp"
#include "algo/quicksort.hpp"
#include "strings/format.hpp"
#include "mem/unique_ptr.hpp"
#include "rendering/vulkan/command_buffers/command.hpp"
#include "rendering/vulkan/command_buffers/command_context.hpp"
#include "rendering/vulkan/operations/operation.hpp"
#include "rendering/vulkan/rendering/reserved_resource.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/node_insertion_context.hpp"
#include "rendering/vulkan/state/satisfied_status.hpp"
#include "rendering/vulkan/state/state_iterator.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"

J_DEFINE_COUNTERS("Vulkan State", state_nodes, merged_nodes, rewinded, forwarded)

#define J_MAKE_STATE_SNAPSHOT(SZ) \
  (::new (J_ALLOCA((SZ) * sizeof(state_variable_iterator) + sizeof(state_snapshot))) state_snapshot)

namespace j::rendering::vulkan::state {
  namespace {
    struct J_TYPE_HIDDEN state_snapshot final {
      i32_t num_satisfied = -1;
      satisfied_status status;

      explicit operator bool() const noexcept {
        return num_satisfied >= 0;
      }

      state_variable_iterator * pos() noexcept {
        return (state_variable_iterator*)((char*)this + sizeof(state_snapshot));
      }

      void update_if_better(const satisfied_status & status, const state_iterator * it) noexcept {
        if (status.has_unsatisfiable()) { return; }
        const i32_t ns = status.num_satisfied();
        if (ns >= num_satisfied) {
          num_satisfied = ns, this->status = status;
          ::j::memcpy(pos(), it->begin(), it->size() * sizeof(state_variable_iterator));
        }
      }
    };

    void handle_unsatisfied_conditions(state_tracker * tracker,
                                       node_insertion_context & context,
                                       const u32_t sz,
                                       const u32_t num_pre,
                                       const u32_t * vars,
                                       const u32_t * const conds,
                                       const u32_t * const cond_flags,
                                       const satisfied_status & status) {
      J_ASSERT_NOT_NULL(tracker, sz, vars, conds);
      for (u32_t i = 0; i < sz; ++i) {
        if (status.is_satisfied(i)) { continue; }
        condition_instance * cond = &tracker->dag->conditions[conds[i]];
        J_ASSERT_NOT_NULL(vars[i]);
        const bool is_pre = i < num_pre;//cond->is_precondition();
        // Re-check, in case a transition added in a previous loop iteration has modified the state,
        // making the current condition valid.
        // Additionally, the failing condition is required, to know where the transition node has
        // to be inserted.
        condition_instance * failing_condition = nullptr;
        state_variable_iterator at(*tracker->dag, vars[i], *cond);
        if (is_pre) {
          at.to_previous(at_condition);
          if (can_insert_precondition_after(cond->as_precondition(), cond_flags[i], at, &failing_condition)) {
            continue;
          }
        } else {
          if (can_insert_postcondition_after(cond->as_postcondition(), cond_flags[i], at, &failing_condition)) {
            continue;
          }
        }
        precondition_instance & transition_target = is_pre
          ? cond->as_precondition() : failing_condition->as_precondition();
        postcondition_instance * const transition_source = is_pre
          ? static_cast<postcondition_instance*>(failing_condition) : &cond->as_postcondition();
        const math::interval_u32 add_range{
          math::by_endpoints,
          transition_source ? transition_source->group->level : 0,
          transition_target.group->level};

        context.begin_add_node();
        condition_group * cmd = tracker->dag->variables[vars[i]].create_transition(
            context, transition_source, transition_target);
        J_ASSERT_NOT_NULL(cmd);
        context.finish_add_node(cmd);
        tracker->add_node(context.render_context, cmd, add_range);
      }
    }

#ifndef NDEBUG
    void assert_node(state_variable_dag & dag, condition_group * node) {
      J_ASSERT(node && node->conditions_index != U16_MAX, "Node must be non-null and have its conditions inserted.");
      const u32_t sz = node->size();
      J_ASSERT(sz && sz < 64, "Node must have 1-64 conditions.");
      J_ASSERT(node->conditions_index + sz <= dag.conditions.size(), "Out of range.");
      for (u32_t i = 0; i < sz; ++i) {
        J_ASSERT(dag.at(node->conditions_index + i).is_precondition() == i < node->num_preconditions,
                 "Condition type mismatch.");
      }
    }
#else
    J_ALWAYS_INLINE void assert_node(const state_variable_dag &, const condition_group *) noexcept { }
#endif
  }
  state_tracker::state_tracker() : dag(::new state_variable_dag) { }
  state_tracker::~state_tracker() { }

  void state_tracker::define_epoch(const epoch_definition & def) noexcept {
    dag->define_epoch(def);
  }

  void state_tracker::add_node(const render_context & context,
                               condition_group * J_NOT_NULL const node,
                               math::interval_u32 allowed_levels)
  {
    mem::unique_ptr<condition_group> node_uniq(node);
    node_insertion_context insert_context{*this, context};
    J_ASSERT_NOT_NULL(allowed_levels);
    J_COUNT(state_nodes);
    assert_node(*dag, node);
    // J_LOG_SCOPED(indent = 2, title = j::strings::format_styled("{#bold}Adding {rainbow}:{/}", node->name()), is_multiline=true);
    bool should_rewind_from_end = false;
    const u32_t sz = node->size();
    for (int i = 0; i < 2; ++i) {
      switch (node->epoch_opts[i]) {
      case epoch_options::no_epoch: break;
      case epoch_options::bound_to_epoch:
        allowed_levels = allowed_levels.intersection(dag->get_epoch_range(node->epoch[i]));
        // J_DEBUG("After bound to epoch: {}", allowed_levels);
        break;
      case epoch_options::starts_epoch:
        allowed_levels.clamp_left(dag->get_last_epoch_level(node->epoch[i]));
        should_rewind_from_end = true;
        // J_DEBUG("After start epoch: {}", allowed_levels);
        break;
      }
    }
    // J_DEBUG("Adding at {}", allowed_levels);
    J_ASSERT_NOT_NULL(allowed_levels);
    J_ASSERT(allowed_levels, "Allowed levels must be non-empty.");

    const u16_t flags = node->insertion_options;
    if ((flags & merges_with_siblings_flag) && (flags & merges_with_parent_flag)) {
      J_ASSERT(node->epoch_opts[0] != epoch_options::starts_epoch && node->epoch_opts[1] != epoch_options::starts_epoch,
               "Mergeable nodes cannot start epochs.");
      auto m = dag->mergeables.maybe_at(type_id::type_id(typeid(*node)));
      if (m) {
        for (auto candidate : *m) {
          J_ASSERT_NOT_NULL(candidate);
          if (allowed_levels.contains(candidate->level) && candidate->can_merge_with(insert_context, *node)) {
            merge_node(insert_context, static_cast<mem::unique_ptr<condition_group> &&>(node_uniq), candidate);
            return;
          }
        }
      }
    }

    const u32_t conditions_index = node->conditions_index;
    J_ASSERT(conditions_index + sz == dag->conditions.size(), "DAG size mismatch Start index: {}, sz={}, dag size={}",
             conditions_index, sz, dag->conditions.size());


    condition_instance * cond_objects[sz];
    u32_t cond_flags[sz];
    u32_t vars[sz];
    const u8_t num_preconditions = node->num_preconditions;
    for (u32_t i = 0; i < sz; ++i) {
      cond_objects[i] = &dag->conditions[conditions_index + i];
      vars[i] = cond_objects[i]->state_variable_index;
      cond_objects[i]->initialize(node);
      cond_flags[i] = i < num_preconditions
        ? cond_objects[i]->as_precondition().definition().flags
        : cond_objects[i]->as_postcondition().definition().flags;
    }

    state_iterator * iterator_set = J_MAKE_STATE_ITERATOR(*dag, sz, vars, should_rewind_from_end);
    if (should_rewind_from_end) {
      J_COUNT(rewinded);
      iterator_set->rewind_to_level(allowed_levels.left());
    } else{
      J_COUNT(forwarded);
      iterator_set->advance_to_level(allowed_levels.left());
    }

    satisfied_status sat_status(insert_context, *iterator_set, conditions_index, cond_flags, num_preconditions);

    state_snapshot * best_status = J_MAKE_STATE_SNAPSHOT(sz);

    math::interval_u32 level_range = iterator_set->level_range_after();
    J_ASSERT(level_range.intersects(allowed_levels), "Level range not satisfied.");
    best_status->update_if_better(sat_status, iterator_set);


    const bool prefers_bottom = flags & prefers_bottom_insert_flag;
    condition_group * g = nullptr;
    while ((prefers_bottom || best_status->num_satisfied < (i32_t)sz) && (g = iterator_set->next_group())) {
      level_range.clamp_left(g->level);
      if (level_range.left() >= allowed_levels.right()) {
        // J_DEBUG("Max level met, bailing.");
        break;
      }
      u8_t i = 0;
      u32_t min_level_after = U32_MAX;
      for (auto & it : *iterator_set) {
        auto ng = it.next_group();
        while (ng == g) {
          it.to_next();
          sat_status.update_condition(i < num_preconditions, insert_context, cond_flags[i], *cond_objects[i], i, it);
          ng = it.next_group(at_condition);
        }
        if (ng) {
          min_level_after = min(min_level_after, ng->level);
        }
        ++i;
      }
      level_range.set_right(min_level_after);
      best_status->update_if_better(sat_status, iterator_set);
    }

    J_ASSERT(*best_status, "Could not resolve node dependencies for {#bright_cyan,bold}{}{/} at {}",
             node->name(), allowed_levels);

    m_nodes.emplace(static_cast<mem::unique_ptr<condition_group> &&>(node_uniq));
    dag->insert_after(node, sz, best_status->pos());
    // J_DEBUG("  Inserted to level {}", node_ptr->level);

    if (best_status->num_satisfied != (i32_t)sz) {
      u32_t conditions[sz];
      for (u8_t i = 0; i < sz; ++i) {
        conditions[i] = i + conditions_index;
      }
      handle_unsatisfied_conditions(this, insert_context, sz, num_preconditions, vars, conditions, cond_flags, best_status->status);
    }
  }

  J_HIDDEN void state_tracker::merge_node(node_insertion_context & context,
                                          mem::unique_ptr<condition_group> command,
                                          condition_group * merge_target) {
    J_COUNT(merged_nodes);
    assert_node(*dag, command.get()); assert_node(*dag, merge_target);


    context.begin_add_node();
    merge_target->merge(context, static_cast<condition_group &&>(*command));
    command.reset(); // Merged, can destroy.
    context.finish_add_node(merge_target);
    assert_node(*dag, merge_target);



    // Count only those conditions that have been added and are uninitialized
    // and detached from the graph.
    u8_t sz = 0;
    // Make an array containing indices to newly added conditions.
    u8_t index_map[merge_target->size()];
    u32_t vars[merge_target->size()];
    u32_t cond_flags[merge_target->size()];
    for (u8_t i = 0, total_sz = merge_target->size(); i < total_sz; ++i) {
      auto & cond = dag->conditions[merge_target->conditions_index + i];
      if (!cond.is_initialized()) {
        index_map[sz] = i;
        vars[sz++] = cond.state_variable_index;
        cond.initialize(merge_target);
      }
    }
    J_ASSERT_NOT_NULL(sz);

    state_iterator * iterator_set = J_MAKE_STATE_ITERATOR(*dag, sz, vars);
    iterator_set->advance_to_level(merge_target->level);
    satisfied_status status;
    // Array containing indices to newly added conditions
    u32_t num_pre = 0;
    u32_t conds[sz];
    for (u8_t i = 0; i < sz; ++i) {
      auto & it = iterator_set->at(i);
      conds[i] = index_map[i] + merge_target->conditions_index;
      auto & cond = dag->conditions[conds[i]];
      dag->handle_added_condition(merge_target, cond, it);
      const bool is_precondition = cond.is_precondition();
      state_variable_iterator it2(*dag, vars[i], cond);
      if (is_precondition) {
        cond_flags[i] = cond.as_precondition().definition().flags;
        ++num_pre;
        it2.rewind_to_previous_group();
      } else {
        cond_flags[i] = cond.as_postcondition().definition().flags;
        it2.to_group_end();
      }
      status.initialize_condition_at(context, it2, cond,
                                     cond_flags[i], i, is_precondition);
    }

    if (status.num_satisfied() != sz) {
      J_ASSERT(!status.has_unsatisfiable(), "Merging node resulted in unsatisfiable conditions.");
      handle_unsatisfied_conditions(this, context, sz, num_pre, vars, conds, cond_flags, status);
    }
  }

  obstack<rendering::reserved_resource> state_tracker::execute(
    command_buffers::command_context & context
  ) {
    J_DUMP_COUNTERS();
    obstack<rendering::reserved_resource> result;
    const u32_t sz = m_nodes.size();
    condition_group * groups[sz];
    {
      condition_group **start = groups;
      for (auto & g : m_nodes) {
        prefetch(*start++ = g.get());
      }
    }


    algo::quicksort(span<condition_group *>(groups, sz),
                    [](const condition_group * J_NOT_NULL g) { return g->level; });

    for (auto g : groups) {
      g->execute(context, result);
    }

    for (auto & n : dag->variables) {
      n.commit(*dag);
    }
    return result;
  }

  void state_tracker::clear() noexcept {
    dag->clear(), m_nodes.clear();
  }
}
