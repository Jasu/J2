#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::inline values {
  struct lisp_object;
}

namespace j::lisp::sources {
  struct source_location;
}
namespace j::lisp::mem {
  class heap_arena;
  struct heap_cell;

  template<bool> struct heap_iterator;

  template<template <bool> typename It, bool IsConst>
  struct heap_iterator_wrapper;

  /// A container-like accessor for a specific type of a heap iterator.
  template<template <bool> typename Iterator>
  struct heap_iterators {
    using const_iterator = heap_iterator_wrapper<Iterator, true>;
    using iterator = heap_iterator_wrapper<Iterator, false>;

    inline const_iterator begin() const noexcept;
    inline const_iterator end() const noexcept;
    inline iterator begin() noexcept;
    inline iterator end() noexcept;
  };

  template<bool> struct arena_value_iterator;
  template<bool> struct arena_free_chunk_iterator;
  template<bool> struct arena_record_iterator;

  using heap_values = heap_iterators<arena_value_iterator>;
  using heap_free_chunks = heap_iterators<arena_free_chunk_iterator>;
  using heap_records = heap_iterators<arena_record_iterator>;

  /// Garbage-collected heap for Lisp values, consisting of one or more arenas.
  class heap final : private heap_values,
                     private heap_free_chunks,
                     private heap_records {
  public:
    using iterator = heap_iterator<false>;
    using const_iterator = heap_iterator<true>;

    heap();

    /// Free a previously allocated pointer.
    void release(heap_cell * J_NOT_NULL ptr) noexcept;

    /// Free a previously allocated pointer.
    J_ALWAYS_INLINE void release(lisp_object * J_NOT_NULL ptr) noexcept
    { release(reinterpret_cast<heap_cell*>(ptr)); }

    /// Debug-dump the heap to console.
    void dump();

    /// Allocate a raw range from the heap.
    [[nodiscard]] heap_cell * allocate(u32_t bytes) noexcept J_RETURNS_NONNULL;

    [[nodiscard]] heap_cell * allocate_with_debug_info(u32_t bytes, const sources::source_location & loc) noexcept;

    /// Get a begin iterator for iterating over all arenas in the heap.
    J_INLINE_GETTER iterator begin() noexcept;
    /// Get an end iterator for iterating over all arenas in the heap.
    J_INLINE_GETTER iterator end() noexcept;
    /// Get a begin iterator for const-iterating over all arenas in the heap.
    J_INLINE_GETTER const_iterator begin() const noexcept;
    /// Get an end iterator for const-iterating over all arenas in the heap.
    J_INLINE_GETTER const_iterator end() const noexcept;

    /// Iterate over all values in the heap, in all arenas.
    J_INLINE_GETTER heap_values & values() noexcept                         { return *this; }
    /// Iterate over all values in the heap, in all arenas.
    J_INLINE_GETTER const heap_values & values() const noexcept             { return *this; }
    /// Iterate over all free chunks in the heap, in all arenas.
    J_INLINE_GETTER heap_free_chunks & free_chunks() noexcept               { return *this; }
    /// Iterate over all free chunks in the heap, in all arenas.
    J_INLINE_GETTER const heap_free_chunks & free_chunks() const noexcept   { return *this; }
    /// Iterate over all regions (values or heap book-keeping) in the heap, in all arenas.
    J_INLINE_GETTER heap_records & records() noexcept                       { return *this; }
    /// Iterate over all regions (values or heap book-keeping) in the heap, in all arenas.
    J_INLINE_GETTER const heap_records & records() const noexcept           { return *this; }

    /// Frees all arenas in the heap.
    ~heap();

    heap(const heap &) = delete;
    heap & operator=(const heap &) = delete;
    heap(heap && rhs) = delete;
    heap & operator=(heap && rhs) = delete;
  private:
    friend heap_values;
    friend heap_free_chunks;
    friend heap_records;

    heap_arena * m_first_arena = nullptr;
  };
}
