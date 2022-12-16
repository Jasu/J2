#pragma once

#include "lisp/air/air_var.hpp"
#include "hzd/iterators.hpp"

namespace j::lisp::air::exprs {
  struct expr;
}

namespace j::lisp::air::inline functions {
  struct air_closure;

  struct activation_record final {
    J_ALWAYS_INLINE activation_record(
      u8_t index,
      i32_t num_closures,
      u32_t num_vars,
      exprs::expr * J_NOT_NULL lowest_scope,
      air_var && var,
      activation_record * parent = nullptr) noexcept
      : num_closures(num_closures),
        num_vars(num_vars),
        index(index),
        parent(parent),
        lowest_scope(lowest_scope),
        act_rec_var(static_cast<air_var &&>(var))
    { }

    i32_t num_closures = 0;
    u32_t num_vars:24 = 0U;
    u32_t index:8 = 0U;

    activation_record * parent = nullptr;
    activation_record * next = nullptr;
    exprs::expr * lowest_scope = nullptr;
    air_var act_rec_var;

    air_closure ** closures_begin() noexcept {
      return reinterpret_cast<air_closure**>(this + 1);
    }

    air_var ** vars_begin() noexcept {
      return add_bytes<air_var**>(this + 1, sizeof(void*) * num_closures);
    }

    [[nodiscard]] span<air_closure *> closures() noexcept {
      return { reinterpret_cast<air_closure**>(this + 1), num_closures };
    }

    [[nodiscard]] span<air_closure * const> closures() const noexcept {
      return { reinterpret_cast<air_closure * const *>(this + 1), num_closures };
    }

    [[nodiscard]] span<air_var *> vars() noexcept {
      return { add_bytes<air_var**>(this + 1, sizeof(void*) * num_closures), num_vars };
    }

    [[nodiscard]] span<air_var * const> vars() const noexcept {
      return { add_bytes<air_var * const *>(this + 1, sizeof(void*) * num_closures), num_vars };
    }

    u32_t get_var_index(const air_var * J_NOT_NULL v) const noexcept {
      u32_t i = 0;
      J_ASSERT(v->activation_record_index == index);
      for (auto var = add_bytes<air_var * const *>(this + 1, sizeof(void*) * num_closures);
           *var != v;
           ++var, ++i) { }
      J_ASSUME(i < num_vars);
      return i;
    }
  };

  using activation_record_iterator = linked_list_iterator<activation_record>;
  using const_activation_record_iterator = linked_list_iterator<const activation_record>;

  struct activation_records final {
    activation_record * first = nullptr;
    activation_record * last = nullptr;
    u32_t m_size = 0U;

    void push_back(activation_record * J_NOT_NULL rec) {
      if (last) {
        J_ASSUME(m_size);
        last->next = rec;
      } else {
        J_ASSUME(first == nullptr);
        J_ASSUME(!m_size);
        first = rec;
      }
      last = rec;
      ++m_size;
    }

    activation_record & find(u32_t i) {
      --i;
      J_ASSUME(i < m_size);
      J_ASSUME_NOT_NULL(last);
      activation_record * cur = first;
      for (; i; --i, cur = cur->next) {
        J_ASSUME_NOT_NULL(cur);
      }
      return *cur;
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return m_size;
    }

    activation_record_iterator begin() noexcept {
      return { first };
    }

    activation_record_iterator end() noexcept {
      return { nullptr };
    }

    const_activation_record_iterator begin() const noexcept {
      return { first };
    }

    const_activation_record_iterator end() const noexcept {
      return { nullptr };
    }
  };
}
