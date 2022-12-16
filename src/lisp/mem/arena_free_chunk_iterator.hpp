#pragma once

#include "lisp/mem/heap_cell.hpp"

namespace j::lisp::mem {
  /// Iterator for the list of free heap chunks.
  template<bool IsConst>
  struct arena_free_chunk_iterator final {
    /// The current node being iterated.
    const_if_t<IsConst, heap_free_chunk> * cur = nullptr;

    J_ALWAYS_INLINE bool operator==(const arena_free_chunk_iterator &) const noexcept = default;

    J_INLINE_GETTER_NONNULL auto * operator->() const noexcept {
      return cur;
    }

    J_INLINE_GETTER auto & operator*() const noexcept {
      return *cur;
    }

    arena_free_chunk_iterator & operator++() noexcept {
      J_ASSUME_NOT_NULL(cur);
      J_ASSERT(cur->is_free_chunk());
      cur = cur->next_free();
      J_ASSERT(!cur || cur->is_free_chunk());
      return *this;
    }

    J_ALWAYS_INLINE arena_free_chunk_iterator operator++(int) noexcept {
      arena_free_chunk_iterator res{cur};
      return operator++(), res;
    }

    /// Allow conversion to a const iterator.
    [[nodiscard]] operator arena_free_chunk_iterator<true>() const noexcept
    { return arena_free_chunk_iterator<true>{cur}; }

    /// Make the iterator empty.
    void reset() noexcept
    { cur = nullptr; }
  };
}
