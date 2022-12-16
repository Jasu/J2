#pragma once

#include "hzd/types.hpp"

namespace j::lisp::mem::code_heap {
  struct code_heap_region final {
    code_heap_region * next_free = nullptr;
    code_heap_region * next = nullptr;
    code_heap_region * prev = nullptr;
    u32_t size = 0U;
    bool is_free = false;
    union {
      code_heap_region * prev_free = nullptr;
      char * target;
    };
  };
}
