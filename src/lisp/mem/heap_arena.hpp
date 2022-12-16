#pragma once

#include "hzd/mem.hpp"

namespace j::mem::vmem {
  class page_range;
}

namespace j::lisp::sources {
  struct source_location;
}

namespace j::lisp::mem {
  struct heap_cell;
  struct heap_free_chunk;

  template<bool> struct arena_value_iterator;
  template<bool> struct arena_free_chunk_iterator;
  template<bool> struct arena_record_iterator;

  template<template <bool> typename Iterator>
  struct arena_iterators {
    using const_iterator = Iterator<true>;
    using iterator = Iterator<false>;

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    iterator begin() noexcept;
    iterator end() noexcept;
  };

  using arena_values = arena_iterators<arena_value_iterator>;
  using arena_records = arena_iterators<arena_record_iterator>;
  using arena_free_chunks = arena_iterators<arena_free_chunk_iterator>;

  template<>
  J_INLINE_GETTER arena_values::iterator arena_values::begin() noexcept;
  template<>
  J_INLINE_GETTER arena_values::iterator arena_values::end() noexcept;
  template<>
  J_INLINE_GETTER arena_values::const_iterator arena_values::begin() const noexcept;
  template<>
  J_INLINE_GETTER arena_values::const_iterator arena_values::end() const noexcept;

  template<>
  J_INLINE_GETTER arena_free_chunks::const_iterator arena_free_chunks::begin() const noexcept;
  template<>
  J_INLINE_GETTER arena_free_chunks::const_iterator arena_free_chunks::end() const noexcept;
  template<>
  J_INLINE_GETTER arena_free_chunks::iterator arena_free_chunks::begin() noexcept;
  template<>
  J_INLINE_GETTER arena_free_chunks::iterator arena_free_chunks::end() noexcept;

  template<>
  J_INLINE_GETTER arena_records::const_iterator arena_records::begin() const noexcept;
  template<>
  J_INLINE_GETTER arena_records::const_iterator arena_records::end() const noexcept;
  template<>
  J_INLINE_GETTER arena_records::iterator arena_records::begin() noexcept;
  template<>
  J_INLINE_GETTER arena_records::iterator arena_records::end() noexcept;

  class alignas(J_PAGE_SIZE) heap_arena final
    : private arena_values,
      private arena_free_chunks,
      private arena_records
  {
  private:
    friend class heap;

    /// Construct a new heap_arena.
    ///
    /// The construction must occur at the beginning of the pages provided.
    heap_arena(j::mem::vmem::page_range && pages, heap_arena * next) noexcept;
  public:

    ~heap_arena();

    heap_cell * allocate_raw(u32_t bytes) noexcept;
    heap_cell * allocate(u32_t bytes) noexcept;
    [[nodiscard]] heap_cell * allocate_with_debug_info(u32_t bytes, const sources::source_location & loc) noexcept;
    void release(heap_cell * J_NOT_NULL ptr) noexcept;

    J_INLINE_GETTER u32_t size() const noexcept { return m_size; }
    J_INLINE_GETTER u32_t capacity() const noexcept
    { return m_size - sizeof(heap_arena) - 2 * sizeof(u64_t); }
    /// Access the next arena.
    J_INLINE_GETTER heap_arena * next() const noexcept
    { return m_next; }
    /// Access the previous arena.
    J_INLINE_GETTER heap_arena * previous() const noexcept
    { return m_previous; }

    /// Enumarate over Lisp values on the heap.
    J_INLINE_GETTER arena_values & values() noexcept { return *this; }
    /// Enumarate over const Lisp values on the heap.
    J_INLINE_GETTER const arena_values & values() const noexcept
    { return const_cast<heap_arena*>(this)->values(); }

    /// Enumarate over free blocks on the heap.
    J_INLINE_GETTER arena_free_chunks & free_chunks() noexcept { return *this; }
    /// Enumarate over free blocks on the heap.
    J_INLINE_GETTER const arena_free_chunks & free_chunks() const noexcept
    { return const_cast<heap_arena*>(this)->free_chunks(); }

    /// Enumarate over all records (values, free blocks, etc.) on the heap.
    J_INLINE_GETTER arena_records & records() noexcept { return *this; }
    /// Enumarate over all records (values, free blocks, etc.) on the heap.
    J_INLINE_GETTER const arena_records & records() const noexcept
    { return const_cast<heap_arena*>(this)->records(); }

    /// Access an iterator set (one of the parent classes) by the iterator template.
    template<template <bool> typename It>
    J_INLINE_GETTER arena_iterators<It> & iterators() noexcept { return *this; }
    /// Access an iterator set (one of the parent classes) by the iterator template.
    template<template <bool> typename It>
    J_INLINE_GETTER const arena_iterators<It> & iterators() const noexcept { return *this; }

    void dump();

    J_INLINE_GETTER_NONNULL heap_cell * region_start() noexcept
    { return reinterpret_cast<heap_cell*>(this + 1); }
    J_INLINE_GETTER_NONNULL heap_cell * region_end() noexcept
    { return reinterpret_cast<heap_cell*>(add_bytes(this, m_size)); }

    heap_arena(const heap_arena &) = delete;
    heap_arena & operator=(const heap_arena &) = delete;
  private:
    [[nodiscard]] heap_cell * first_value() noexcept J_RETURNS_NONNULL;
    J_INLINE_GETTER_NONNULL const heap_cell * first_value() const noexcept
    { return const_cast<heap_arena*>(this)->first_value(); }

    [[nodiscard]] heap_cell * end_sentinel() noexcept J_RETURNS_NONNULL;
    J_INLINE_GETTER_NONNULL const heap_cell * end_sentinel() const noexcept
    { return const_cast<heap_arena*>(this)->end_sentinel(); }

    friend arena_values;
    friend arena_free_chunks;
    friend arena_records;

    /// Size of the heap arena in bytes, including allocated memory and overhead.
    u32_t m_size = 0U;
    /// First free object, or null if none.
    heap_free_chunk * m_first_free_chunk = nullptr;
    /// Next heap arena, or null if none.
    heap_arena * m_next = nullptr;
    /// Previous heap arena, or null if none.
    heap_arena * m_previous = nullptr;
  };
}
