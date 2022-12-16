#pragma once

#include "lisp/mem/heap.hpp"
#include "lisp/mem/heap_arena.hpp"
#include "lisp/mem/arena_value_iterator.hpp"
#include "lisp/mem/arena_free_chunk_iterator.hpp"
#include "lisp/mem/arena_record_iterator.hpp"
#include "lisp/mem/heap_iterator.hpp"

namespace j::lisp::mem {
  template<>
  J_INLINE_GETTER arena_values::iterator arena_values::begin() noexcept
  { return iterator{static_cast<heap_arena*>(this)->first_value()}; }
  template<>
  J_INLINE_GETTER arena_values::iterator arena_values::end() noexcept
  { return iterator{static_cast<heap_arena*>(this)->end_sentinel()}; }
  template<>
  J_INLINE_GETTER arena_values::const_iterator arena_values::begin() const noexcept
  { return const_cast<arena_values*>(this)->begin(); }
  template<>
  J_INLINE_GETTER arena_values::const_iterator arena_values::end() const noexcept
  { return const_cast<arena_values*>(this)->end(); }


  template<>
  J_INLINE_GETTER arena_free_chunks::const_iterator arena_free_chunks::begin() const noexcept
  { return const_cast<arena_free_chunks*>(this)->begin(); }
  template<>
  J_INLINE_GETTER arena_free_chunks::const_iterator arena_free_chunks::end() const noexcept
  { return const_cast<arena_free_chunks*>(this)->end(); }

  template<>
  J_INLINE_GETTER arena_free_chunks::iterator arena_free_chunks::begin() noexcept {
    return iterator{static_cast<heap_arena*>(this)->m_first_free_chunk};
  }
  template<>
  J_INLINE_GETTER arena_free_chunks::iterator arena_free_chunks::end() noexcept {
    return iterator{nullptr};
  }


  template<>
  J_INLINE_GETTER arena_records::iterator arena_records::begin() noexcept {
    return iterator{static_cast<heap_arena*>(this)->region_start()};
  }
  template<>
  J_INLINE_GETTER arena_records::const_iterator arena_records::begin() const noexcept
  { return const_cast<arena_records*>(this)->begin(); }
  template<>
  J_INLINE_GETTER arena_records::iterator arena_records::end() noexcept {
    return iterator{static_cast<heap_arena*>(this)->region_end()};
  }
  template<>
  J_INLINE_GETTER arena_records::const_iterator arena_records::end() const noexcept
  { return const_cast<arena_records*>(this)->end(); }



  heap::iterator heap::begin() noexcept
  { return heap::iterator{m_first_arena}; }
  heap::const_iterator heap::begin() const noexcept
  { return heap::iterator{m_first_arena}; }
  heap::iterator heap::end() noexcept
  { return heap::iterator{nullptr}; }
  heap::const_iterator heap::end() const noexcept
  { return heap::iterator{nullptr}; }



  template<template <bool> typename It>
  inline typename heap_iterators<It>::const_iterator heap_iterators<It>::begin() const noexcept
  { return heap_iterators<It>::const_iterator(static_cast<heap*>(this)->begin()); }
  template<template <bool> typename It>
  inline typename heap_iterators<It>::const_iterator heap_iterators<It>::end() const noexcept
  { return heap_iterators<It>::iterator(static_cast<heap*>(this)->end()); }
  template<template <bool> typename It>
  inline typename heap_iterators<It>::iterator heap_iterators<It>::begin() noexcept
  { return heap_iterators<It>::iterator(static_cast<heap*>(this)->begin()); }
  template<template <bool> typename It>
  inline typename heap_iterators<It>::iterator heap_iterators<It>::end() noexcept
  { return heap_iterators<It>::iterator(static_cast<heap*>(this)->end()); }
}
