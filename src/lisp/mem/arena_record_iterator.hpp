#pragma once

#include "lisp/mem/heap_cell.hpp"

namespace j::lisp::mem {
  /// Iterator for iterating over values and heap book-keeping objects in a heap.
  template<bool IsConst>
  struct arena_record_iterator final {
    const_if_t<IsConst, heap_cell> * cur = nullptr;

    J_INLINE_GETTER_NONNULL auto * operator->() const noexcept {
      return cur;
    }

    J_INLINE_GETTER auto & operator*() const noexcept
    { return *cur; }

    J_ALWAYS_INLINE bool operator==(const arena_record_iterator &) const noexcept = default;

    J_ALWAYS_INLINE arena_record_iterator & operator++() noexcept {
      J_ASSUME_NOT_NULL(cur);
      cur = cur->next();
      return *this;
    }

    J_ALWAYS_INLINE arena_record_iterator operator++(int) noexcept {
      arena_record_iterator result{*this};
      return operator++(), result;
    }

    /// Conversion to const iterator
    operator arena_record_iterator<true>() const noexcept
    { return {cur}; }

    /// Make the iterator empty.
    void reset() noexcept { cur = nullptr; }
  };
}
