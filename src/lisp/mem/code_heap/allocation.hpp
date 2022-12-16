#pragma once

#include "lisp/mem/code_heap/subheap_allocation.hpp"

namespace j::lisp::mem::code_heap {
  struct code_heap;
  struct allocation final {
    subheap_allocation code_allocation;
    subheap_allocation data_allocation;
    code_heap * heap = nullptr;

    J_A(AI,NODISC) const void * code_begin() const noexcept {
      return code_allocation.chr ? code_allocation.chr->target : nullptr;
    }
    J_A(AI,NODISC) const void * data_begin() const noexcept {
      return data_allocation.chr ? data_allocation.chr->target : nullptr;
    }

    [[nodiscard]] j::mem::memory_region code_region() const noexcept {
      return code_allocation.region();
    }

    [[nodiscard]] j::mem::memory_region data_region() const noexcept {
      return data_allocation.region();
    }

    void assign_code(j::mem::const_memory_region code);
    void assign_data(j::mem::const_memory_region data);

    void assign(j::mem::const_memory_region code, j::mem::const_memory_region data) {
      assign_code(code);
      if (data) {
        assign_data(data);
      }
    }

    J_A(AI,ND) inline allocation() noexcept = default;

    J_A(AI,ND) inline allocation(code_heap * J_NOT_NULL heap, code_heap_region * J_NOT_NULL code, code_heap_region * data) noexcept
      : code_allocation{code},
        data_allocation{data},
        heap(heap)
    {
    }

    J_A(AI,ND) inline allocation(allocation && rhs) noexcept
      : code_allocation(rhs.code_allocation),
        data_allocation(rhs.data_allocation),
        heap(rhs.heap)
    {
      rhs.heap = nullptr;
    }

    J_ALWAYS_INLINE allocation & operator=(allocation && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        code_allocation.chr = rhs.code_allocation.chr;
        data_allocation.chr = rhs.data_allocation.chr;
        heap = rhs.heap;
        rhs.code_allocation.chr = nullptr;
        rhs.data_allocation.chr = nullptr;
        rhs.heap = nullptr;
      }
      return *this;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool empty() const noexcept {
      return !heap;
    }
    J_A(AI,ND,HIDDEN,NODISC) inline bool operator!() const noexcept {
      return !heap;
    }
    J_A(AI,ND,HIDDEN,NODISC) inline explicit operator bool() const noexcept {
      return heap;
    }

    void release() noexcept;

    ~allocation() {
      if (heap) {
        release();
      }
    }
  };
}
