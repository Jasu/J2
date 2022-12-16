#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "mem/bump_pool.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN lex_vars_visitor final {
      air_var ** vars;
      void visit(exprs::expr * J_NOT_NULL e) noexcept {
        if (e->type == expr_type::let || e->type == expr_type::fn_body) {
          auto defs = reinterpret_cast<air_var_defs_t*>(e->static_begin());
          if (defs->size()) {
            auto first_index = (*defs)[0]->index;
            auto cur = defs->begin();
            for (u32_t i = 0; i < defs->size(); ++i) {
              vars[first_index + i] = *cur++;
            }
          }
        }
        for (auto & in : e->inputs()) {
          visit(in.expr);
        }
      }
    };

    void apply_lex_vars_pass(air_pass_context & ctx) noexcept {
      const u32_t num_vars = ctx.fn->num_lex_vars;
      if (!num_vars) {
        return;
      }
      air_var ** vars = (air_var**)ctx.pool->allocate(num_vars * sizeof(air_var*));
      lex_vars_visitor v{vars};
      v.visit(ctx.fn->body);
      ctx.fn->lex_vars = span(vars, num_vars);

      u32_t i = 0U;
      for (auto p : ctx.fn->lex_vars) {
        if (p) {
          J_ASSERT(p->index == i);
        }
        ++i;
      }
    }
  }

  const air_pass lex_vars_pass{apply_lex_vars_pass, "lex-vars", "Gather lex vars to array", true};
}
