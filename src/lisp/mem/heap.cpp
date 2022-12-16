#include "lisp/mem/heap.hpp"
#include "lisp/mem/heap_arena.hpp"
#include "lisp/mem/heap_cell.hpp"
#include "lisp/mem/iteration.hpp"
#include "logging/global.hpp"
#include "mem/page_pool.hpp"
#include "strings/format.hpp"

namespace j::lisp::mem {
  heap::heap() {
    j::mem::vmem::page_range range = j::mem::vmem::page_pool::instance.allocate(32U);
    m_first_arena = ::new (range.address()) heap_arena(static_cast<j::mem::vmem::page_range &&>(range), nullptr);
  }

  heap::~heap() {
    for (heap_arena * arena = m_first_arena; arena; arena = arena->next()) {
      const u32_t size = arena->size();
      J_ASSUME(size >= J_PAGE_SIZE);
      J_ASSUME((size & J_PAGE_SIZE_MASK) == 0);
      arena->~heap_arena();
      j::mem::vmem::page_pool::instance.release(arena, size);
    }
  }

  J_RETURNS_NONNULL heap_cell * heap::allocate(u32_t bytes) noexcept {
    for (heap_arena * arena = m_first_arena; arena; arena = arena->next()) {
      heap_cell * result = arena->allocate(bytes);
      if (result) {
        return result;
      }
    }
    j::mem::vmem::page_range range = j::mem::vmem::page_pool::instance.allocate(8U);
    m_first_arena = ::new (range.address()) heap_arena(static_cast<j::mem::vmem::page_range &&>(range), m_first_arena);
    heap_cell * result = m_first_arena->allocate(bytes);
    J_ASSERT_NOT_NULL(result);
    return result;
  }
  [[nodiscard]] heap_cell * heap::allocate_with_debug_info(u32_t bytes, const sources::source_location & loc) noexcept {
    for (heap_arena * arena = m_first_arena; arena; arena = arena->next()) {
      heap_cell * result = arena->allocate_with_debug_info(bytes, loc);
      if (result) {
        return result;
      }
    }
    j::mem::vmem::page_range range = j::mem::vmem::page_pool::instance.allocate(8U);
    m_first_arena = ::new (range.address()) heap_arena(static_cast<j::mem::vmem::page_range &&>(range), m_first_arena);
    heap_cell * result = m_first_arena->allocate_with_debug_info(bytes, loc);
    J_ASSERT_NOT_NULL(result);
    return result;
  }

  void heap::release(heap_cell * J_NOT_NULL ptr) noexcept {
    for (heap_arena * arena = m_first_arena; arena; arena = arena->next()) {
      if (ptr < (heap_cell*)arena) {
        continue;
      }
      if (ptr < add_bytes<heap_cell*>(arena, 8U * J_PAGE_SIZE)
          || ptr < arena->region_end()) {
        arena->release(ptr);
        return;
      }
    }
    J_THROW("Pointer not found in arenas.");
  }

  void heap::dump() {
    {
      J_LOG_SCOPED(
        title = strings::format_styled("{#black,bright_yellow_bg,bold} Heap free block iteration test {/}")
      );
      for (auto & r : free_chunks()) {
        J_DEBUG("{#bright_red,bold}Free block {}", (void*)&r.value(), r.value());
      }
    }
    {
      J_LOG_SCOPED(
        title = strings::format_styled("{#bright_yellow,gray_bg,bold} Heap value iteration test {/}")
        );
      for (auto & v : values()) {
        J_DEBUG("{#bright_red,bold}Value {}", (void*)&v.value(), static_cast<const heap_cell &>(v.value()));
      }
    }

    {
      J_LOG_SCOPED(
        title = strings::format_styled("\n{#black,white_bg,bold} Iterating all heap entries {/}")
      );
      for (auto & h : records()) {
        J_DEBUG("{#bright_yellow,bold}  0x{:06X}:{/} {}",
                byte_offset(h.arena().region_start(), &h.value()),
                static_cast<const heap_cell&>(h.value()));
        if (h.value().has_footer()) {
          J_DEBUG("{#bright_magenta,bold}  0x{:06X}:{/} {}", byte_offset(h.arena().region_start(), &h.value().footer()), h.value().footer());
        }
      }
    }
    // for (heap_arena * arena = m_first_arena; arena; arena = arena->next()) {
    //   arena->dump();
    // }
  }
}
