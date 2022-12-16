#include "lisp/mem/heap_arena.hpp"
#include "lisp/mem/heap_cell.hpp"
#include "lisp/mem/iteration.hpp"
#include "mem/page_range.hpp"
#include "logging/global.hpp"
#include "strings/format.hpp"

namespace j::lisp::mem {
  namespace {
    inline void add_free(heap_cell * J_NOT_NULL start,
                         heap_cell * J_NOT_NULL end,
                         heap_free_chunk * J_NOT_NULL next) noexcept {
      J_ASSUME(start < end);
      J_ASSERT((u64_t)next <= (u64_t)start || (u64_t)next > (u64_t)end, "Next free range overlaps region.");
      u32_t sz = end - start;
      ::new (start) heap_free_chunk(sz, next);
      if (sz > 1U) {
        ::new (start + sz - 1) heap_footer(start, true);
      }
    }

    [[nodiscard]] J_RETURNS_NONNULL inline heap_cell * allocate_from(heap_free_chunk * J_NOT_NULL block,
                                                                     u32_t block_sz, u32_t sz) noexcept {
      J_ASSERT(block->size_taken() == block_sz);
      J_ASSUME(block_sz > sz);
      J_ASSUME((block_sz & 7) == 0);
      J_ASSUME(block_sz >= 16U);
      J_ASSUME(sz >= 8U);
      J_ASSUME((sz & 7) == 0);
      heap_cell * result = reinterpret_cast<heap_cell*>(add_bytes(block, block_sz - sz));
      add_free(reinterpret_cast<heap_cell*>(block), result, block->next_free());
      if (sz > 8U) {
        ::new (add_bytes(result, sz - 8U)) heap_footer(result, false);
      }
      return result;
    }

    [[nodiscard]] inline heap_free_chunk * find_prev(
      heap_free_chunk * J_NOT_NULL first_free,
      heap_free_chunk * J_NOT_NULL chunk) noexcept {
      heap_free_chunk * cur = first_free, *next = cur ? cur->next_free() : nullptr;
      while (cur && next != chunk) {
        cur = next;
        next = cur ? cur->next_free() : nullptr;
      }
      return cur;
    }
  }

  heap_arena::heap_arena(j::mem::vmem::page_range && pages, heap_arena * next) noexcept
    : m_size(pages.size()),
      m_next(next)
  {
    J_ASSERT(pages.address() == this, "Invalid page range");
    J_ASSERT(next != this, "Next cannot be this.");
    if (next) {
      next->m_previous = this;
    }
    heap_cell * start = region_start();
    heap_cell * const end = region_end() - 1;
    ::new (start++) heap_sentinel(false);
    add_free(start, end, (heap_free_chunk*)start);
    ::new (end) heap_sentinel(true);
    m_first_free_chunk = &start->as_free_chunk();
    pages.reset_no_free();
  }

  heap_arena::~heap_arena() {
    J_ASSUME(m_size >= J_PAGE_SIZE);
  }

  heap_cell * heap_arena::allocate_raw(u32_t sz) noexcept {
    J_ASSUME(sz != 0U);
    const u32_t aligned_sz = align_up(sz, 8U) + (sz > 8U ? 8U : 0U);
    heap_free_chunk * previous = nullptr;
    for (heap_free_chunk & chunk : free_chunks()) {
      const u32_t chunk_sz = chunk.size_taken();
      J_ASSUME(chunk_sz >= 8U);

      // No fit, continue
      if (chunk_sz < aligned_sz) {
        previous = &chunk;
        continue;
      }

      // Block is larger, allocate from its end.
      if (chunk_sz > aligned_sz) {
        return allocate_from(&chunk, chunk_sz, aligned_sz);
      }

      // Perfect fit
      heap_free_chunk * const next = chunk.next_free();
      if (previous) {
        previous->set_next_free(next);
      } else {
        m_first_free_chunk = next;
      }

      return reinterpret_cast<heap_cell*>(&chunk);
    }
    return nullptr;
  }

  heap_cell * heap_arena::allocate(u32_t sz) noexcept {
    heap_cell * result = allocate_raw(sz);
    if (!result) {
      return nullptr;
    }
    if (sz > 8U) {
      // Need footer
      ::new (result + (align_up(sz, 8) >> 3)) heap_footer(result, false);
    }
    return result;
  }

  [[nodiscard]] heap_cell * heap_arena::allocate_with_debug_info(u32_t bytes, const sources::source_location & loc) noexcept {
    u32_t new_sz = bytes <= 8U ? bytes + 16U : bytes + 24U;
    heap_cell * result = allocate_raw(new_sz);
    if (!result) {
      return nullptr;
    }
    ::new (result) heap_debug_info(source_location_tag, loc);
    ::new (result + 2) heap_footer(result, false);
    result += 3U;
    if (bytes > 8U) {
      // Need footer
      ::new (result + (align_up(bytes, 8) >> 3)) heap_footer(result, false);
    }
    return result;
  }

  void heap_arena::release(heap_cell * J_NOT_NULL ptr) noexcept {
    heap_cell * prev = ptr->previous(), * next = ptr->next();
    J_ASSUME(prev < ptr);
    J_ASSUME(ptr < next);
    const bool prev_free = prev->is_free_chunk(), next_free = next->is_free_chunk();
    if (!prev_free && !next_free) {
      add_free(ptr, next,
               m_first_free_chunk ? m_first_free_chunk->next_free() : (heap_free_chunk*)ptr);
      if (m_first_free_chunk) {
        m_first_free_chunk->set_next_free(&ptr->as_free_chunk());
      }
      m_first_free_chunk = &ptr->as_free_chunk();
    } else if (!next_free) {
      add_free(prev, next, prev->as_free_chunk().next_free());
    } else {
      heap_free_chunk * prev_free_reg = find_prev(m_first_free_chunk, &next->as_free_chunk());
      if (prev_free) {
        add_free(reinterpret_cast<heap_cell*>(prev_free_reg), reinterpret_cast<heap_cell*>(prev_free_reg->next()), prev_free_reg);
        add_free(prev, next->next(), next->as_free_chunk().next_free());
      } else {
        add_free(reinterpret_cast<heap_cell*>(prev_free_reg), reinterpret_cast<heap_cell*>(prev_free_reg->next()), &ptr->as_free_chunk());
        add_free(ptr, next->next(), next->as_free_chunk().next_free());
      }
      m_first_free_chunk = prev_free_reg;
    }
  }

  [[nodiscard]] J_RETURNS_NONNULL heap_cell * heap_arena::first_value() noexcept {
    heap_cell * start = region_start();
    J_ASSERT(start->is_sentinel());
    ++start;
    while (start->is_debug_info()) {
      start = start->next();
    }
    if (start->is_object()) {
      return start;
    }
    J_ASSERT(start->is_free_chunk());
    start = start->next();
    while (start->is_debug_info()) {
      start = start->next();
    }
    J_ASSERT(start->is_object() || start->is_sentinel());
    return start;
  }

  [[nodiscard]] J_RETURNS_NONNULL heap_cell * heap_arena::end_sentinel() noexcept {
    heap_cell * end = region_end() - 1;
    J_ASSERT(end->is_sentinel());
    return end;
  }

  void heap_arena::dump() {
    J_LOG_SCOPED(
      indent = 2,
      title = j::strings::format_styled(
        "{#bright_yellow,bold}Heap arena at 0x{:12X}, size = 0x{:6X}{/}",
        (uptr_t)this,
        m_size
      ),
      is_multiline=true);

    {
      J_LOG_SCOPED(
        title = strings::format_styled("\n{#black,bright_yellow_bg,bold} Arena free block iteration test {/}")
      );
      for (auto & r : free_chunks()) {
        J_DEBUG("{#bright_red,bold}Free block {}", (uptr_t)&r, (const heap_cell&)r);
      }
    }
    {
      J_LOG_SCOPED(
        title = strings::format_styled("\n{#bright_yellow,gray_bg,bold} Arena value iteration test {/}\n")
        );
      for (auto & v : values()) {
        J_DEBUG("{#bright_red,bold}Value {}", (uptr_t)&v, (const heap_cell &)v);
      }
    }

    {
      J_LOG_SCOPED(
        title = strings::format_styled("\n{#black,white_bg,bold} Iterating arena entries {/}\n")
      );
      for (auto & h : records()) {
        J_DEBUG("{#bright_yellow,bold}  0x{:06X}:{/} {}", byte_offset(region_start(), &h), (const heap_cell &)h);
        if (h.has_footer()) {
          J_DEBUG("{#bright_magenta,bold}  0x{:06X}:{/} {}", byte_offset(region_start(), &h.footer()), h.footer());
        }
      }
    }
  }
}
