#pragma once

#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/condition_group.hpp"
#include "rendering/vulkan/state/state_variable.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "debug/counters.hpp"

J_DEFINE_COUNTERS("Vulkan State", rewind_steps, forward_steps, level_steps)

namespace j::rendering::vulkan::state {
  enum class at_condition_tag { v };
  enum class at_end_tag { v };
  enum class at_begin_tag { v };
  enum class at_unknown_tag { v };

  constexpr inline at_condition_tag at_condition = at_condition_tag::v;
  constexpr inline at_end_tag at_end = at_end_tag::v;
  constexpr inline at_begin_tag at_begin = at_begin_tag::v;
  constexpr inline at_unknown_tag at_unknown = at_unknown_tag::v;

  /// Iterates conditions in a state variable.
  class state_variable_iterator final {
  public:
    constexpr state_variable_iterator() noexcept = default;

    /// Construct a state variable iterator at start of var.
    explicit state_variable_iterator(state_variable_dag & dag, u32_t var_index, bool at_end = false) noexcept
      : m_dag(&dag),
        m_it(get_index(var_index) | (at_end ? variable_tail_mask_v : variable_head_mask_v)),
        m_var_index(var_index)
    {
      J_ASSERT(m_var_index < dag.variables.size(), "Variable index out of range.");
    }

    /// Construct a state variable iterator pointing to condition cond.
    explicit state_variable_iterator(state_variable_dag & dag, u32_t var_index, condition_instance & cond) noexcept
      : m_dag(&dag),
        m_it(&cond - dag.conditions.begin()),
        m_var_index(get_index(var_index))
    {
      J_ASSERT(m_dag->conditions.size() > m_it, "Condition out of range.");
    }

    /// Get the index of the next node.
    J_INLINE_GETTER u32_t next_index() const noexcept {
      J_ASSERT(!points_to_variable_tail(m_it), "Tried to forward past end.");
      return points_to_condition(m_it)
        ? m_dag->conditions[m_it].next
        : m_dag->variables[m_it].head;
    }

    /// Get the index of the next node.
    J_INLINE_GETTER u32_t next_index(at_begin_tag) const noexcept {
      J_ASSERT(points_to_variable_head(m_it), "at_begin_tag not at begin");
      return m_dag->variables[m_it].head;
    }

    /// Get the index of the previous node.
    J_INLINE_GETTER u32_t previous_index() const noexcept {
      J_ASSERT(!points_to_variable_head(m_it), "Tried to rewind past begin.");
      return points_to_condition(m_it)
        ? m_dag->conditions[m_it].previous
        : m_dag->variables[m_it].tail;
    }

    /// Get the index of the next node.
    J_INLINE_GETTER u32_t previous_index(at_end_tag) const noexcept {
      J_ASSERT(points_to_variable_tail(m_it), "at_end_tag not at end");
      return m_dag->variables[m_it].tail;
    }

    J_INLINE_GETTER bool is_beginning() const noexcept {
      return points_to_variable_head(m_it);
    }

    J_INLINE_GETTER bool is_end() const noexcept {
      return points_to_variable_tail(m_it);
    }

    J_INLINE_GETTER bool is_at_condition() const noexcept {
      return points_to_condition(m_it);
    }

    J_INLINE_GETTER bool is_precondition() const noexcept {
      return points_to_condition(m_it) && !m_dag->conditions[m_it].is_postcondition();
    }

    J_INLINE_GETTER bool is_precondition(at_condition_tag) const noexcept {
      return !m_dag->conditions[m_it].is_postcondition();
    }

    J_INLINE_GETTER bool is_postcondition() const noexcept {
      return points_to_condition(m_it) && m_dag->conditions[m_it].is_postcondition();
    }

    J_INLINE_GETTER bool is_postcondition(at_condition_tag) const noexcept {
      return m_dag->conditions[m_it].is_postcondition();
    }

    /// Rewind to the previous condition.
    J_ALWAYS_INLINE void to_previous() noexcept {
      J_ASSERT(!is_beginning(), "Cannot rewind from beginning.");
      m_it = previous_index();
    }

    J_ALWAYS_INLINE void to_previous(at_end_tag) noexcept {
      m_it = m_dag->variables[m_it].tail;
    }

    J_ALWAYS_INLINE void to_previous(at_condition_tag) noexcept {
      m_it = m_dag->conditions[m_it].previous;
    }

    /// Forward to the next condition.
    J_ALWAYS_INLINE void to_next() noexcept {
      J_ASSERT(!is_end(), "Cannot advance from end.");
      m_it = points_to_condition(m_it)
        ? m_dag->conditions[m_it].next
        : m_dag->variables[m_it].head;
    }

    /// Forward to the next condition.
    J_ALWAYS_INLINE void to_next(at_condition_tag) noexcept {
      m_it = m_dag->conditions[m_it].next;
    }

    /// Forward to the next condition.
    J_ALWAYS_INLINE void to_next(at_begin_tag) noexcept {
      m_it = m_dag->variables[m_it].head;
    }

    J_ALWAYS_INLINE state_variable_iterator & operator++() noexcept {
      return to_next(), *this;
    }
    J_ALWAYS_INLINE state_variable_iterator operator++(int) noexcept {
      state_variable_iterator result{*this};
      return to_next(), result;
    }

    J_ALWAYS_INLINE state_variable_iterator & operator--() noexcept {
      return to_previous(), *this;
    }
    J_ALWAYS_INLINE state_variable_iterator operator--(int) noexcept {
      state_variable_iterator result{*this};
      return to_previous(), result;
    }

    /// Forward to the start of the next condition group.
    template<typename Tag = at_unknown_tag>
    void to_previous_postcondition(Tag = at_unknown) noexcept {
      J_ASSERT(!is_beginning(), "Cannot rewind from beginning.");
      if constexpr (is_same_v<Tag, at_condition_tag>) {
        m_it = m_dag->conditions[m_it].previous;
      } else {
        m_it = previous_index();
      }
      for (; points_to_condition(m_it) && !m_dag->conditions[m_it].is_postcondition();
           m_it = m_dag->conditions[m_it].previous)
      { }
    }

    /// Forward to the start of the next condition group.
    void to_next_postcondition() noexcept {
      J_ASSERT(!is_end(), "Cannot get next group at end.");
      for (m_it = next_index();
           points_to_condition(m_it) && !m_dag->conditions[m_it].is_postcondition();
           m_it = m_dag->conditions[m_it].next)
      { }
    }

    /// Rewind to the end of the previous condition group.
    void rewind_to_previous_group() noexcept {
      J_ASSERT(!is_beginning(), "Cannot rewind from beginning.");
      const condition_group * const g = m_dag->conditions[m_it].group;
      do {
        m_it = m_dag->conditions[m_it].previous;
      } while (points_to_condition(m_it) && m_dag->conditions[m_it].group == g);
    }

    /// Forward to the start of the next condition group.
    void to_next_group() noexcept {
      J_ASSERT(!is_end(), "Cannot get next group at end.");
      auto g = group();
      if (J_UNLIKELY(!g)) {
        J_ASSERT_STATE_IDX_POINTS_TO_VAR_HEAD(m_it);
        to_next(at_begin);
        return;
      }
      do {
        m_it = m_dag->conditions[m_it].next;
      } while (points_to_condition(m_it) && m_dag->conditions[m_it].group == g);
    }

    /// Forward to the start of the next condition group.
    void to_next_group(at_condition_tag) noexcept {
      J_ASSERT(!is_end(), "Cannot get next group at end.");
      condition_group * g = group(at_condition);
      do {
        m_it = m_dag->conditions[m_it].next;
      } while (points_to_condition(m_it) && m_dag->conditions[m_it].group == g);
    }

    /// Forward to the end of the current condition group.
    template<typename Tag = at_unknown_tag>
    void to_group_end(Tag tag = at_unknown) noexcept {
      J_ASSERT(!is_end(), "Cannot get next group at end.");
      auto g = group(tag);
      if constexpr (!is_same_v<Tag, at_condition_tag>) {
        if (!g) {
          return;
        }
      } else {
        J_ASSUME(g != nullptr);
      }
      u32_t n = m_dag->conditions[m_it].next;
      while (points_to_condition(n) && m_dag->conditions[n].group == g) {
        m_it = n;
        n = m_dag->conditions[n].next;
      }
    }

    /// Forward to the condition instance to the level specified, or just before it.
    void advance_to_level(u32_t level) noexcept {
      u32_t prev = m_it;
      u32_t n = next_index(at_begin);
      while (points_to_condition(n)) {
        J_COUNT(forward_steps);
        J_COUNT(level_steps);
        auto & cond = m_dag->conditions[n];
        if (cond.group->level > level) {
          break;
        }
        prev = n;
        n = cond.next;
      }
      m_it = prev;
      J_ASSERT(!is_end(), "Cannot advance from end.");
    }

    /// Rewind to the condition instance to the level specified, or just before it.
    void rewind_to_level(u32_t level) noexcept {
      u32_t n = previous_index(at_end);
      while (points_to_condition(n)) {
        J_COUNT(rewind_steps);
        J_COUNT(level_steps);
        auto & cond = m_dag->conditions[n];
        if (cond.group->level <= level) {
          break;
        }
        n = cond.previous;
      }
      m_it = n;
      J_ASSERT(!is_end(), "Ended up at end after rewind.");
      J_ASSERT(this->level() <= level, "Level mismatch");
      J_ASSERT(next_level() > this->level(), "NOt at moundary.");
    }

    J_INLINE_GETTER condition_instance & operator*() const noexcept {
      return m_dag->conditions[m_it];
    }

    J_INLINE_GETTER_NONNULL condition_instance * operator->() const noexcept {
      return &m_dag->conditions[m_it];
    }

    template<typename Tag = at_unknown_tag>
    J_INLINE_GETTER condition_group * group(Tag = at_unknown) const noexcept {
      if constexpr (!is_same_v<Tag, at_condition_tag>) {
        if (!points_to_condition(m_it)) {
          return nullptr;
        }
      }
      auto * const g = m_dag->conditions[m_it].group;
      J_ASSUME(g != nullptr);
      return g;
    }

    J_INLINE_GETTER condition_group * next_group() const noexcept {
      J_ASSERT(!is_end(), "Cannot get next group at end.");
      const u32_t next = points_to_condition(m_it)
        ? m_dag->conditions[m_it].next
        : m_dag->variables[m_it].head;
      if (points_to_condition(next)) {
        auto group = m_dag->conditions[next].group;
        J_ASSUME(group != nullptr);
        return group;
      } else {
        return nullptr;
      }
    }

    J_INLINE_GETTER condition_group * next_group(at_condition_tag) const noexcept {
      const u32_t next = m_dag->conditions[m_it].next;
      return points_to_condition(next) ? m_dag->conditions[next].group : nullptr;
    }

    J_INLINE_GETTER state_variable & var() const noexcept {
      J_ASSERT_NOT_NULL(m_dag);
      return m_dag->variables[m_var_index];
    }

    J_INLINE_GETTER u32_t index() const noexcept
    { return m_it; }

    J_ALWAYS_INLINE void set_position(condition_instance & it) noexcept {
      J_ASSERT_NOT_NULL(m_dag);
      m_it = &it - m_dag->conditions.begin();
      J_ASSERT(m_it < m_dag->conditions.size(), "Position out of range.");
    }

    J_ALWAYS_INLINE void set_index(u32_t it) noexcept {
      m_it = it;
    }

    J_INLINE_GETTER u32_t level() const noexcept {
      if (points_to_condition(m_it)) {
        return m_dag->conditions[m_it].group->level;
      }
      return points_to_variable_head(m_it) ? 0U : U32_MAX;
    }

    J_INLINE_GETTER u32_t next_level() const noexcept {
      u32_t next = next_index();
      return points_to_condition(next) ? m_dag->conditions[next].group->level : U32_MAX;
    }
  private:
    state_variable_dag * m_dag = nullptr;
    /// Pointer to the current condition.
    u32_t m_it = U16_MAX;
    /// Pointer to the state variable being iterated.
    u32_t m_var_index = U16_MAX;
  };
}
