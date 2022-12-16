#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/passes/air_pass.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/expr.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    using namespace exprs;
    struct J_A(HIDDEN) count_exprs_visitor final {
      air_function * fn = nullptr;

      void visit(expr * J_NOT_NULL e) {
        fn->counts.add_expr(e->type);
        for (auto & in : e->inputs()) {
          if (in.expr) {
            visit(in.expr);
          }
        }
      }
    };

    void count_exprs(air_pass_context & ctx) {
      count_exprs_visitor v{ctx.fn};
      ctx.fn->counts.reset();
      v.visit(ctx.fn->body);
    }
  }

  const air_pass count_exprs_pass{count_exprs, "count-exprs", "Count exprs"};
}
