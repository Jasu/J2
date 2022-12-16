#include "lisp/assembly/asm_context.hpp"

namespace j::lisp::assembly {
  asm_ctx::asm_ctx(const class target * J_NOT_NULL target) noexcept
    : target(target) {
  }

  J_A(NODESTROY) thread_local j::util::context_stack<asm_ctx> asm_context{};
}
