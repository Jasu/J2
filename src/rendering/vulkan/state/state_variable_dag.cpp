#include "rendering/vulkan/state/state_variable_dag.hpp"

#include "rendering/vulkan/state/condition_group.hpp"
#include "rendering/vulkan/state/state_iterator.hpp"
#include "containers/hash_map.hpp"
#include "containers/vector.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::rendering::vulkan::state::condition_group*);
J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::ptr_array<j::rendering::vulkan::state::condition_group*>, HASH(j::type_id::hash));

namespace j::rendering::vulkan::state {
  state_variable_dag::state_variable_dag() noexcept {
  }

  state_variable_dag::~state_variable_dag() {
    clear();
  }

  void state_variable_dag::add_precondition(const precondition_initializer & c) {
    u32_t idx = variables.get_or_create(c.key, c.variable_definition);
    conditions.emplace(c, idx);
  }

  void state_variable_dag::add_postcondition(const postcondition_initializer & c) {
    u32_t idx = variables.get_or_create(c.key, c.variable_definition);
    conditions.emplace(c, idx);
  }

  condition_instance * state_variable_dag::next_of(u32_t idx) noexcept {
    J_ASSERT(!points_to_variable_tail(idx), "Next of tail.");
    const u32_t next_idx = points_to_condition(idx)
      ? conditions[idx].next
      : variables[idx].head;
    return points_to_condition(next_idx) ? &conditions[next_idx] : nullptr;
  }

  condition_instance * state_variable_dag::next_of(const condition_instance * g) const noexcept {
    J_ASSERT_NOT_NULL(g);
    return points_to_condition(g->next) ? &conditions[g->next] : nullptr;
  }

  u32_t state_variable_dag::next_index_of(u32_t idx) const noexcept {
    if (points_to_condition(idx)) {
      return conditions[idx].next;
    }
    J_ASSERT_STATE_IDX_POINTS_TO_VAR_HEAD(idx);
    return variables[idx].head;
  }

  void state_variable_dag::move_condition(condition_instance && from) {
    const u32_t new_index = conditions.size();
    if (points_to_condition(from.next)) {
      conditions[from.next].previous = new_index;
    } else if (points_to_variable(from.next)) {
      variables[from.next].tail = new_index;
    }
    if (points_to_condition(from.previous)) {
      conditions[from.previous].next = new_index;
    } else if (points_to_variable(from.previous)) {
      variables[from.previous].head = new_index;
    }
    condition_instance from_{static_cast<condition_instance &&>(from)};
    conditions.emplace(static_cast<condition_instance &&>(from_));
  }

  void state_variable_dag::finish_add_node(condition_group * const node, const u32_t index) noexcept {
    J_ASSERT_NOT_NULL(node);
    const u32_t sz = conditions.size();
    u32_t num_pre = 0, num_post = 0;
    condition_instance * last_postcondition = nullptr;
    for (u32_t i = index; i != sz; ++i) {
      auto & cond = conditions[i];
      if (cond.is_precondition()) {
        ++num_pre;
        // Swap nodes so that preconditions always come before postconditions.
        if (last_postcondition) {
          condition_instance i(static_cast<condition_instance &&>(*last_postcondition));
          *last_postcondition = static_cast<condition_instance &&>(cond);
          cond = static_cast<condition_instance &&>(i);
          last_postcondition = &cond;
        }
      } else {
        J_ASSERT(cond.is_postcondition(), "Unknown condition type");
        ++num_post;
        last_postcondition = &cond;
      }
    }
    node->num_preconditions = num_pre, node->num_postconditions = num_post;
    node->conditions_index = index;
  }

  void state_variable_dag::define_epoch(const epoch_definition & def) noexcept {
    J_ASSERT(epochs[def.index].definition.name == nullptr, "Epoch was defined twice.");
    epochs[def.index].definition = def;
  }

  void state_variable_dag::get_state_variables(state_variable ** dst,
                                               [[maybe_unused]] u32_t count) {
    J_ASSERT_NOT_NULL(dst);
    J_ASSERT(count >= variables.size(), "Destination array too small.");
    for (auto & var : variables) {
      *dst++ = &var;
    }
  }

  void state_variable_dag::insert_after(condition_group * group, u32_t num, state_variable_iterator * pos) {
    J_ASSERT_NOT_NULL(group, num, pos);
    J_ASSERT(group->size() == num, "Number of insert positions must be same for group and position.");
    const u32_t conditions_index = group->conditions_index;

    u32_t max_level = 0;
    for (u32_t i = 0; i < num; ++i) {
      state_variable_iterator & p = pos[i];
      J_ASSERT(!p.is_end(), "Cannot insert past end.");
      auto & cond = conditions[conditions_index + i];

      if (cond.is_postcondition()) {
        while (p.next_group() == group) {
          // Inserting multiple conditions for the same state variable.
          // NOTE: The variable must be incremented manually once, since if the
          // condition group adds multiple nodes to the beginning of the variable,
          // the insert pointer will be on a node that has NULL group.
          // NOTE 2: This modifies the caller's iterator by pointer.
          p.to_next(at_condition);
        }
      }
      max_level = max(max_level, p.level());

      cond.previous = p.index();
      cond.next = next_index_of(p.index());
      attach_node(&cond, group->conditions_index + i);
    }

    if (group->insertion_options & (merges_with_siblings_flag | merges_with_parent_flag)) {
      mergeables[type_id::type_id(typeid(*group))].push_back(group);
    }

    u32_t level_increment = 1U;
    for (int i = 0; i < 2; ++i) {
      if (group->epoch_opts[i] == epoch_options::no_epoch) {
        continue;
      }

      J_ASSERT_EPOCH_RANGE(group->epoch[i].type());
      auto & epoch = epochs[group->epoch[i].type()];
      const u32_t nuepochs = epoch.epoch_starters.size();


      if (group->epoch_opts[i] == epoch_options::starts_epoch) {
        max_level = ::j::max(max_level, epoch.end_level);
        // Not typo
        level_increment = (2U * (nuepochs + 1U)) << epoch.definition.level_shift;
        // Remove the previous per-epoch bitmask from the path.
        max_level &= epoch.definition.level_mask_inv;
        max_level |= (2U * nuepochs + 1U) << epoch.definition.level_shift;
        group->epoch[i] = epoch_key(group->epoch[i].type(), nuepochs + 1);
        // J_WARNING("Creating epoch #{}", group->epoch[i].counter());
        epoch.epoch_starters.emplace_back(group);
      } else {
        if (group->epoch[i].is_end()) {
          group->epoch[i] = epoch_key(group->epoch[i].type(), nuepochs);
        }
        if (group->epoch[i].counter()) {
          max_level = j::max(max_level, epoch.epoch_starters[group->epoch[i].counter() - 1]->level);
        }
        // Remove the previous per-epoch bitmask from the path.
        max_level &= epoch.definition.level_mask_inv;
        max_level |= (2U * group->epoch[i].counter()) << epoch.definition.level_shift;
      }
    }
    update_node_levels(max_level + 1, group, level_increment);
  }

  void state_variable_dag::handle_added_condition(condition_group * group, condition_instance & cond, state_variable_iterator pos) {
    J_ASSERT_NOT_NULL(group);
    J_ASSERT(cond.group == group, "Condition group mismatch.");
    J_ASSERT(cond.next == U16_MAX && cond.previous == U16_MAX,
             "Condition already in graph.");
    if (group == pos.group()) {
      if (cond.is_precondition()) {
        while (pos->is_postcondition() && pos->group == group) {
          pos.to_previous(at_condition);
        }
      } else {
        pos.to_group_end(at_condition);
      }
    } else if (pos.next_level() <= group->level) {
      J_ASSERT(group->level == pos.next_level(), "Level mismatch.");
      update_node_levels(group->level + 1U, pos.next_group(), 1U);
    }

    cond.previous = pos.index();
    cond.next = next_index_of(cond.previous);
    attach_node(&cond, &cond - conditions.begin());
  }

  void state_variable_dag::clear() noexcept {
    for (u32_t i = 0U; i < max_epoch_types_v; ++i) {
      epochs[i].epoch_starters.resize(0);
      epochs[i].end_level = 0U;
    }
    variables.clear();
    conditions.clear(mem::keep_allocation);
    mergeables.clear();
  }

  J_INTERNAL_LINKAGE void state_variable_dag::update_node_levels(u32_t level, condition_group * group, u32_t level_increment) {
    J_ASSERT_NOT_NULL(group);
    J_ASSERT(level > group->level, "Tried to set level to lower or equal value.");
    group->level = level;

    for (int i = 0; i < 2; ++i) {
      if (group->epoch_opts[i] == epoch_options::no_epoch) {
        continue;
      }
      J_ASSERT_EPOCH_RANGE(group->epoch[i].type());
      auto & epoch = epochs[group->epoch[i].type()];
      for (u32_t j = group->epoch[i].counter(), sz = epoch.epoch_starters.size(); j < sz; ++j) {
        auto c = epoch.epoch_starters[j];
        J_ASSERT_NOT_NULL(c);
        if (c->level < level + level_increment) {
          update_node_levels(level + level_increment, c, level_increment);
          break;
        }
      }
      epoch.end_level = max(epoch.end_level, level);
    }

    for (u32_t i = group->conditions_index, end = i + group->size(); i != end; ++i) {
      auto n = &conditions[i];
      if (!points_to_condition(n->next)) {
        continue;
      }
      n = &conditions[n->next];
      if (n->group == group) {
        continue;
      }
      if (n->group->level <= level) {
        update_node_levels(level + 1, n->group, level_increment);
      }
    }
  }

  J_INTERNAL_LINKAGE void state_variable_dag::attach_node(condition_instance * cond, u32_t index) noexcept {
    J_ASSERT_NOT_NULL(cond);
    if (points_to_variable(cond->next)) {
      J_ASSERT_STATE_IDX_POINTS_TO_VAR_TAIL(cond->next);
      variables[cond->next].tail = index;
    } else {
      conditions[cond->next].previous = index;
    }
    if (points_to_variable(cond->previous)) {
      J_ASSERT_STATE_IDX_POINTS_TO_VAR_HEAD(cond->previous);
      variables[cond->previous].head = index;
    } else {
      conditions[cond->previous].next = index;
    }
  }

  math::interval_u32 state_variable_dag::get_epoch_range(epoch_key key) const noexcept {
    J_ASSERT_EPOCH_RANGE(key.type());
    auto & epoch = epochs[key.type()];
    if (key.is_begin()) {
      return {
        math::by_endpoints,
        0,
        epoch.epoch_starters.empty() ? U32_MAX : epoch.epoch_starters.front()->level };
    } else if (key.is_end()) {
      return {
        math::by_endpoints,
        epoch.epoch_starters.empty() ? 0U : epoch.epoch_starters.back()->level,
        U32_MAX };
    } else {
      const auto i = key.counter();
      return {
        math::by_endpoints,
        epoch.epoch_starters.at(i - 1)->level,
        epoch.epoch_starters.size() == i ? U32_MAX : epoch.epoch_starters.at(i)->level,
      };
    }
  }
}
