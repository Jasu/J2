#include "lisp/mem/code_heap/subheap.hpp"
#include "lisp/mem/code_heap/code_heap_region.hpp"
#include "hzd/mem.hpp"

namespace j::lisp::mem::code_heap {
  namespace {
    void unlink_free(subheap * J_NOT_NULL heap, code_heap_region * J_NOT_NULL chr) {
      if (chr->prev_free) {
        J_ASSUME(chr->prev_free->next_free == chr);
        chr->prev_free->next_free = chr->next_free;
      } else {
        J_ASSUME(heap->first_free == chr);
        heap->first_free = chr->next_free;
      }
      if (chr->next_free) {
        J_ASSUME(chr->next_free->prev_free == chr);
        chr->next_free->prev_free = chr->prev_free;
      } else {
        J_ASSUME(heap->last_free == chr);
        heap->last_free = chr->prev_free;
      }
    }
  }

  void subheap_allocation::release(subheap * J_NOT_NULL heap) noexcept {
    J_ASSUME_NOT_NULL(chr);
    auto next = chr->next, prev = chr->prev;
    J_ASSERT(!next || next->prev == chr);
    J_ASSERT(!prev || prev->next == chr);
    bool next_is_free = next && next->is_free;
    bool prev_is_free = prev && prev->is_free;
    if (next_is_free) {
      chr->size += next->size;
      unlink_free(heap, next);
      chr->next = next->next;
      if (!chr->next) {
        heap->last = chr;
      }
      ::delete next;
    }
    if (prev_is_free) {
      chr->size += prev->size;
      chr->target = prev->target;
      unlink_free(heap, prev);
      chr->prev = prev->prev;
      if (!chr->prev) {
        heap->first = chr;
      }
      ::delete prev;
    }
    chr->next_free = heap->first_free;
    chr->prev_free = nullptr;
    if (chr->next_free) {
      J_ASSUME(chr->next_free != chr);
      chr->next_free->prev_free = chr;
    } else {
      J_ASSUME(heap->last_free == nullptr);
      heap->last_free = chr;
    }
    heap->first_free = chr;
    chr = nullptr;
  }

  void subheap::initialize(void * J_NOT_NULL target, u32_t size) {
    J_ASSUME(size > 0);
    J_ASSUME(capacity == 0);
    capacity = size;
    J_ASSUME(first == nullptr);
    J_ASSUME(last == nullptr);
    J_ASSUME(first_free == nullptr);
    J_ASSUME(last_free == nullptr);
    first = ::new code_heap_region{
      .size = size,
      .is_free = true,
      .target = (char*)target,
    };
    last = first;
    first_free = first;
    last_free = first;
  }


  subheap::~subheap() {
    for (code_heap_region *it = first; it;) {
      auto next = it->next;
      ::delete (it);
      it = next;
    }
  }

  code_heap_region * subheap::allocate(u32_t size) {
    J_ASSUME(size > 0);
    size = j::align_up(size, 64);
    code_heap_region * best_fit = nullptr;
    for (auto free = first_free; free; free = free->next_free) {
      J_ASSUME(free->is_free);
      J_ASSUME(free->size >= 64);
      if (free->size < size) {
        continue;
      }
      if (!best_fit || best_fit->size > free->size) {
        best_fit = free;
        if (best_fit->size == size) {
          break;
        }
      }
    }
    J_REQUIRE(best_fit, "Out of code heap memory.");
    if (best_fit->size == size) {
      best_fit->is_free = false;
      unlink_free(this, best_fit);
      return best_fit;
    }

    code_heap_region * new_region = ::new code_heap_region{
      .next = best_fit,
      .prev = best_fit->prev,
      .size = size,
      .is_free = false,
      .target = best_fit->target,
    };
    best_fit->size -= size;
    best_fit->target = add_bytes(best_fit->target, size);
    best_fit->prev = new_region;
    if (!new_region->prev) {
      first = new_region;
    } else {
      new_region->prev->next = new_region;
    }

    return new_region;
  }
}
