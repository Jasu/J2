#pragma once

#include "lisp/mem/code_heap/subheap_allocation.hpp"

namespace j::lisp::mem::code_heap {
  struct subheap final {
    code_heap_region * first = nullptr;
    code_heap_region * last = nullptr;
    code_heap_region * first_free = nullptr;
    code_heap_region * last_free = nullptr;
    u32_t capacity = 0;

    constexpr subheap() noexcept = default;
    subheap(const subheap &) = delete;
    subheap & operator=(const subheap &) = delete;

    ~subheap();

    void initialize(void * J_NOT_NULL target, u32_t size);

    [[nodiscard]] code_heap_region * allocate(u32_t size);
  };

}
