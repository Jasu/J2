#pragma once

#include "lisp/mem/code_heap/code_heap_region.hpp"
#include "mem/memory_region.hpp"

namespace j::lisp::mem::code_heap {
  struct code_heap_region;
  struct subheap;

  struct subheap_allocation final {
    J_A(AI,NODISC,HIDDEN) inline j::mem::memory_region region() const noexcept {
      return { chr ? chr->target : nullptr, chr ? chr->size : 0U };
    }

    J_A(AI,NODISC,HIDDEN,ND) inline bool empty() const noexcept {
      return !chr;
    }
    J_A(AI,NODISC,HIDDEN,ND) inline bool operator!() const noexcept {
      return !chr;
    }
    J_A(AI,NODISC,HIDDEN,ND) inline explicit operator bool() const noexcept {
      return chr;
    }

    void release(subheap * J_NOT_NULL heap) noexcept;

    code_heap_region * chr = nullptr;
  };
}
