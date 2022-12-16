#pragma once

#include "containers/hash_map_fwd.hpp"
#include "containers/vector.hpp"
#include "containers/ptr_set.hpp"
#include "type_id/type_hash.hpp"
#include "math/interval.hpp"
#include "mem/typed_arena.hpp"
#include "rendering/vulkan/state/conditions.hpp"
#include "rendering/vulkan/state/epochs.hpp"
#include "rendering/vulkan/state/state_variable.hpp"
#include "rendering/vulkan/state/state_variable_table.hpp"

namespace j::rendering::vulkan::state {
  struct state_variable_comparer;
}

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::rendering::vulkan::state::condition_group*);
J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::ptr_array<j::rendering::vulkan::state::condition_group*>, HASH(j::type_id::hash));

namespace j::rendering::vulkan::state {
  class state_variable_iterator;

  class state_variable_dag final {
  public:
    state_variable_dag() noexcept;
    ~state_variable_dag();

    condition_instance * next_of(const condition_instance * g) const noexcept;
    condition_instance * next_of(u32_t idx) noexcept;

    u32_t next_index_of(u32_t idx) const noexcept;

    void add_precondition(const precondition_initializer & precondition);
    void add_postcondition(const postcondition_initializer & postcondition);

    void move_condition(condition_instance && from);
    void finish_add_node(condition_group * node, u32_t start_index) noexcept;

    void define_epoch(const epoch_definition & def) noexcept;

    J_INLINE_GETTER condition_instance & at(u32_t i) noexcept {
      return conditions.at(i);
    }
    J_INLINE_GETTER const condition_instance & at(u32_t i) const noexcept {
      return const_cast<state_variable_dag*>(this)->at(i);
    }

    void get_state_variables(state_variable ** dst, u32_t count);

    /// Insert group into the graph after conditions specified in pos.
    void insert_after(condition_group * group, u32_t num, state_variable_iterator * pos);

    /// Insert cond at pos, possibly bumping next nodes.
    void handle_added_condition(condition_group * group, condition_instance & cond, state_variable_iterator pos);

    math::interval_u32 get_epoch_range(epoch_key key) const noexcept;

    /// Gets the last level that belongs to an epoch.
    u32_t get_last_epoch_level(epoch_key key) const noexcept {
      J_ASSERT_EPOCH_RANGE(key.type());
      return epochs[key.type()].end_level;
    }

    J_INLINE_GETTER u32_t variable_index(state_variable * var) const noexcept {
      J_ASSERT_NOT_NULL(var);
      J_ASSERT(variables.size() > (var - variables.end()), "Invalid variable ptr");
      return var - variables.begin();
    }

    void clear() noexcept;

    state_variable_dag(const state_variable_dag &) = delete;
    state_variable_dag & operator=(const state_variable_dag &) = delete;
  private:
    J_INTERNAL_LINKAGE void update_node_levels(u32_t level, condition_group * group, u32_t level_increment);
    J_INTERNAL_LINKAGE void attach_node(condition_instance * cond, u32_t index) noexcept;

    struct epoch_record final {
      noncopyable_vector<condition_group *> epoch_starters;
      u32_t end_level = 0U;
      epoch_definition definition;
    };

  public:
    mem::typed_arena<condition_instance> conditions;
    state_variable_table variables;
    epoch_record epochs[max_epoch_types_v];
    hash_map<type_id::type_id, ptr_array<condition_group*>, type_id::hash> mergeables;
  };
}
