#pragma once

#include "lisp/mem/heap.hpp"
#include "lisp/mem/heap_arena.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::lisp::mem {
  /// Iterator for iterating over heap_arenas of a heap.
  template<bool IsConst>
  struct heap_iterator final {
    const_if_t<IsConst, heap_arena> * cur = nullptr;

    J_INLINE_GETTER_NONNULL auto * operator->() const noexcept {
      return cur;
    }

    J_INLINE_GETTER auto & operator*() const noexcept
    { return *cur; }

    J_ALWAYS_INLINE bool operator==(const heap_iterator &) const noexcept = default;

    heap_iterator & operator++() noexcept {
      J_ASSUME_NOT_NULL(cur);
      cur = cur->next();
      return *this;
    }

    heap_iterator operator++(int) noexcept {
      heap_iterator result{*this};
      return operator++(), result;
    }

    J_ALWAYS_INLINE explicit operator bool() const noexcept
    { return cur; }
    J_ALWAYS_INLINE bool operator!() const noexcept
    { return !cur; }

    /// Conversion to const iterator
    operator heap_iterator<true>() const noexcept
    { return {cur}; }
  };

  template<template <bool> typename It, bool IsConst>
  struct heap_iterator_state {
    It<IsConst> it, end;
    heap_iterator<IsConst> heap_it;

    J_ALWAYS_INLINE constexpr heap_iterator_state() noexcept = default;
    J_ALWAYS_INLINE heap_iterator_state(It<IsConst> it, It<IsConst> end, heap_iterator<IsConst> heap_it) noexcept
      : it(it),
        end(end),
        heap_it(heap_it)
    { }

    explicit heap_iterator_state(heap_iterator<IsConst> heap_it) noexcept
      : heap_it(heap_it)
    {
      if (this->heap_it) {
        it = this->heap_it->template iterators<It>().begin();
        end = this->heap_it->template iterators<It>().end();
        if (it == end) {
          next_arena();
        }
      }
    }

    void next_arena() noexcept {
      while (it == end) {
        if (!++heap_it) {
          it.reset();
          end.reset();
          return;
        } else {
          it = heap_it->template iterators<It>().begin();
          end = heap_it->template iterators<It>().end();
        }
      }
    }


    [[nodiscard]] inline decltype(auto) value() const noexcept {
      J_ASSERT(it != end);
      return *it;
    }
    J_INLINE_GETTER decltype(auto) arena() const noexcept { return *heap_it; }
  };

  template<template <bool> typename It, bool IsConst>
  struct heap_iterator_wrapper final : private heap_iterator_state<It, IsConst> {
    using heap_iterator_state<It, IsConst>::heap_iterator_state;
    using heap_iterator_state<It, IsConst>::it;
    using heap_iterator_state<It, IsConst>::end;
    using heap_iterator_state<It, IsConst>::heap_it;
    using heap_iterator_state<It, IsConst>::next_arena;

    J_ALWAYS_INLINE constexpr heap_iterator_wrapper() noexcept = default;

    // J_ALWAYS_INLINE explicit heap_iterator_wrapper(heap_iterator<IsConst> heap_it) noexcept {
    //   if (heap_it) {
    //     this->heap_it = heap_it;
    //     it = heap_it->template iterators<It>().begin();
    //     end = heap_it->template iterators<It>().end();
    //   }
    // }

    J_INLINE_GETTER_NONNULL const heap_iterator_state<It, IsConst> * operator->() const noexcept {
      J_ASSERT(it != end && (bool)heap_it);
      return this;
    }
    J_INLINE_GETTER const heap_iterator_state<It, IsConst> & operator*() const noexcept {
      J_ASSERT(it != end && (bool)heap_it);
      return *this;
    }

    J_ALWAYS_INLINE bool operator==(const heap_iterator_wrapper & rhs) const noexcept
    { return it == rhs.it; }

    [[nodiscard]] heap_iterator_wrapper & operator++() noexcept {
      J_ASSERT(it != end && (bool)heap_it);
      if (++it == end) {
        next_arena();
      }
      return *this;
    }

    J_ALWAYS_INLINE heap_iterator_wrapper operator++(int) noexcept {
      heap_iterator_wrapper result{*this};
      return operator++(), result;
    }

    /// Conversion to const iterator
    operator heap_iterator_wrapper<It, true>() const noexcept
    { return heap_iterator_wrapper<It, true>(heap_it, it, end); }
  };
}
