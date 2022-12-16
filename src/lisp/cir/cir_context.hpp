#pragma once

#include "util/context_stack.hpp"
#include "lisp/cir/target/cir_target.hpp"

namespace j::lisp::cir {
  struct cir_function;
  struct cir_ctx final {
    explicit cir_ctx(const assembly::target * J_NOT_NULL tgt);
    cir_target target;
  };

  extern thread_local j::util::context_stack<cir_ctx> cir_context;
}
