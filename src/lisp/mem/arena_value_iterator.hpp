#pragma once

#include "lisp/mem/heap_cell.hpp"

namespace j::lisp::mem {
  /// Iterator for iterating over values in a heap.
  template<bool IsConst>
  struct arena_value_iterator final {
    using heap_cell_t = const_if_t<IsConst, heap_cell>;

    heap_cell_t * cur = nullptr;

    J_INLINE_GETTER_NONNULL heap_cell_t * operator->() const noexcept {
      return cur;
    }

    J_INLINE_GETTER heap_cell_t & operator*() const noexcept
    { return *cur; }

    J_ALWAYS_INLINE bool operator==(const arena_value_iterator &) const noexcept = default;

    arena_value_iterator & operator++() noexcept {
      J_ASSERT(cur->is_object());
      cur = cur->next();
      if (cur->is_free_chunk()) {
        cur = cur->next();
      }
      while (cur->is_debug_info()) {
        cur = cur->next();
      }
      J_ASSERT(cur->is_object() || cur->is_sentinel());
      return *this;
    }

    J_ALWAYS_INLINE arena_value_iterator operator++(int) noexcept {
      arena_value_iterator result{*this};
      return operator++(), result;
    }

    J_ALWAYS_INLINE operator arena_value_iterator<true>() const noexcept {
      return {cur};
    }

    /// Make the iterator empty.
    void reset() noexcept { cur = nullptr; }
  };
}
