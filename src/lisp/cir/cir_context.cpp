#include "lisp/cir/cir_context.hpp"

namespace j::lisp::cir {
  J_A(NODESTROY) thread_local j::util::context_stack<cir_ctx> cir_context;

  cir_ctx::cir_ctx(const assembly::target * J_NOT_NULL tgt)
    : target(tgt)
  {
  }
}
