#pragma once

#include "rendering/vulkan/state/state_variable_dag.hpp"
#include "rendering/vulkan/state/state_variable_iterator.hpp"
#include "hzd/mem.hpp"
#include "math/interval.hpp"

namespace j::rendering::vulkan::state {
  class state_variable_dag;

  /// Iterator for iterating over multiple state variables.
  class state_iterator final {
  public:
    static sz_t get_size(state_variable_dag & dag) noexcept;
    constexpr static sz_t get_size(state_variable_dag &, u32_t size, u32_t *, bool = false) noexcept {
      return size * sizeof(state_variable_iterator) + sizeof(state_iterator);
    }

    state_iterator(state_variable_dag & dag) noexcept;

    state_iterator(state_variable_dag & dag, u32_t size, u32_t * state_variable_indices, bool at_end = false) noexcept
      : m_size(size)
    {
      J_ASSERT_NOT_NULL(state_variable_indices);
      state_variable_iterator * it = begin();
      for (u32_t i = 0U; i < m_size; ++i) {
        ::new (it++) state_variable_iterator(dag, *state_variable_indices++, at_end);
      }
    }

    state_iterator(const state_iterator &) = delete;
    state_iterator(state_iterator &&) = delete;
    state_iterator & operator=(const state_iterator &) = delete;
    state_iterator & operator=(state_iterator &&) = delete;

    J_INLINE_GETTER u32_t size() const noexcept { return m_size; }
    J_INLINE_GETTER state_variable_iterator & at(u8_t iterator_index) noexcept {
      J_ASSERT(iterator_index < m_size, "Out of range.");
      return begin()[iterator_index];
    }
    J_INLINE_GETTER const state_variable_iterator & at(u8_t iterator_index) const noexcept {
      return const_cast<state_iterator*>(this)->at(iterator_index);
    }
    J_INLINE_GETTER state_variable_iterator * begin() noexcept {
      return (state_variable_iterator*)(this + 1);
    }
    J_INLINE_GETTER const state_variable_iterator * begin() const noexcept {
      return (const state_variable_iterator*)(this + 1);
    }

    J_INLINE_GETTER state_variable_iterator * end() noexcept {
      return begin() + m_size;
    }
    J_INLINE_GETTER const state_variable_iterator * end() const noexcept {
      return begin() + m_size;
    }

    math::interval_u32 level_range_after() const noexcept {
      math::interval_u32 result(math::by_endpoints, 0U, U32_MAX);
      for (auto & it : *this) {
        result.clamp_left(it.level());
        result.clamp_right(it.next_level());
      }
      return result;
    }

    u32_t level_range_after_end() const noexcept {
      u32_t result = U32_MAX;
      for (auto & it : *this) {
        result = ::j::min(result, it.next_level());
      }
      return result;
    }

    condition_group * next_group() const noexcept {
      u32_t min_level = U32_MAX;
      condition_group * min_group = nullptr;
      for (auto & it : *this) {
        condition_group * const g = it.next_group();
        if (g && min_level > g->level) {
          min_group = g, min_level = g->level;
        }
      }
      return min_group;
    }

    condition_group * advance_to_next_group() noexcept {
      u32_t min_level = U32_MAX;
      u32_t prev_cond = U32_MAX;
      condition_group * min_group = nullptr;
      for (auto & it : *this) {
        if (it.index() == prev_cond) {
          continue;
        }
        prev_cond = it.index();

        condition_group * const g = it.next_group();
        if (g && g == min_group) {
        } else if (g && min_level > g->level) {
          min_group = g, min_level = g->level;
        }
      }
      if (!min_group) {
        return min_group;
      }

      for (auto & it : *this) {
        if (it.next_group() == min_group) {
          it.to_next();
          while (it.next_group(at_condition) == min_group) {
            it.to_next(at_condition);
          }
        }
      }

      return min_group;
    }

    void advance_to_level(u32_t level) noexcept {
      if (level) {
        for (auto & it : *this) {
          it.advance_to_level(level);
        }
      }
    }

    void rewind_to_level(u32_t level) noexcept {
      for (auto & it : *this) {
        it.rewind_to_level(level);
      }
    }
  private:
    u32_t m_size;
  };
}

#define J_MAKE_STATE_ITERATOR(...)                                               \
    (::new (J_ALLOCA(::j::rendering::vulkan::state::state_iterator::get_size(__VA_ARGS__))) \
      ::j::rendering::vulkan::state::state_iterator(__VA_ARGS__))
