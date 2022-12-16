#pragma once

#include "lisp/mem/code_heap/subheap.hpp"
#include "lisp/mem/code_heap/allocation.hpp"

namespace j::lisp::mem::code_heap {
  struct code_heap final {
    code_heap(u32_t code_size, u32_t data_size);
    ~code_heap();

    allocation allocate(u32_t code_size, u32_t data_size);

    subheap code;
    subheap data;

    void * heap;
  };
}
