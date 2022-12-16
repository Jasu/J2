#pragma once

#include "util/context_stack.hpp"

namespace j::lisp::assembly {
  struct asm_ctx final {
    asm_ctx(const class target * J_NOT_NULL target) noexcept;
    const class target * target;
  };

  extern thread_local j::util::context_stack<asm_ctx> asm_context;
}
