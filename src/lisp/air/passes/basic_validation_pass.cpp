#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/passes/air_pass.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "logging/global.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN expr_validation_data final {
      u8_t min_inputs = 0U;
      u8_t max_inputs = 255U;
    };

#define J_VDATA(E, ...) [(u8_t)expr_type::E] = { __VA_ARGS__ }
#define NULLARY .max_inputs = 0U
#define UNARY .min_inputs = 1U, .max_inputs = 1U
#define BINARY .min_inputs = 2U, .max_inputs = 2U
#define TRINARY .min_inputs = 3U, .max_inputs = 3U
#define NUM_INS(MIN,MAX) .min_inputs = (MIN), .max_inputs = (MAX)
#define MIN_INS(MIN) .min_inputs = (MIN)
#define MAX_INS(MIN) .max_inputs = (MIN)

    constexpr inline expr_validation_data validation_data[num_expr_types_v]{
      J_VDATA(nop, NULLARY),
      J_VDATA(ld_const, NULLARY),
      J_VDATA(fn_arg, NULLARY),
      J_VDATA(fn_rest_arg, NULLARY),
      J_VDATA(fn_sc_arg, NULLARY),
      J_VDATA(lex_rd, NULLARY),
      J_VDATA(lex_rd_bound, NULLARY),
      J_VDATA(lex_wr, UNARY),
      J_VDATA(lex_wr_bound, UNARY),
      J_VDATA(sym_val_rd, NULLARY),
      J_VDATA(sym_val_wr, UNARY),
      J_VDATA(type_assert, UNARY),
      J_VDATA(dyn_type_assert, UNARY),
      J_VDATA(ineg, UNARY),
      J_VDATA(b_if, NUM_INS(2, 3)),
      J_VDATA(do_until, UNARY),
      J_VDATA(eq, BINARY),
      J_VDATA(neq, BINARY),
      J_VDATA(call, MIN_INS(1)),
      J_VDATA(tag, UNARY),
      J_VDATA(untag, UNARY),
      J_VDATA(as_range, UNARY),
      J_VDATA(as_vec, UNARY),
      J_VDATA(make_vec, UNARY),
      J_VDATA(range_length, UNARY),
      J_VDATA(range_get, BINARY),
      J_VDATA(range_set, TRINARY),
      J_VDATA(range_copy, UNARY),
      J_VDATA(act_rec_alloc, UNARY),
      J_VDATA(act_rec_wr, BINARY),
      J_VDATA(act_rec_rd, UNARY),
      J_VDATA(act_rec_parent, UNARY),
      J_VDATA(closure, BINARY),
    };

    using namespace exprs;
    struct J_A(HIDDEN) basic_validation_visitor final {
      compilation::context & ctx;
      air_function * fn = nullptr;
      expr_counts counts{};
      expr * cur_expr = nullptr;
      i8_t child_index = -1;

      template<typename... Args>
      void fail(const char * J_NOT_NULL message, Args && ... args) {
        J_ERROR("{#bright_red_bg,bright_yellow,bold} Validation failed {/}");
        J_ERROR(" {#bright_red,bold}{}{/}", strings::format(message, static_cast<Args &&>(args)...));
        J_ERROR(" {#bright_yellow,bold}For expr {}{/}", cur_expr->type);

        expr * p = cur_expr->parent;
        for (u8_t i = 0; i < 10 && p; ++i, p = p->parent) {
          J_ERROR("   {#bright_yellow}Below {}{/}", p->type);
        }
        J_ERROR(" {#bright_yellow,bold}In function {}{/}", ctx.name());
        // J_THROW("Validation failed");
      }

      template<typename... Args>
      void check(bool result, const char * J_NOT_NULL message, Args && ... args) {
        if (!result) {
          fail(message, static_cast<Args &&>(args)...);
        }
      }

      void visit(expr * J_NOT_NULL e) {
        counts.add_expr(e->type);
        child_index = -1;
        cur_expr = e;
        check(e->num_inputs <= e->max_inputs, "Too many inputs");
        check(e->visit_flag == fn->visit_flag_value, "Visit flag mismatch");
        auto & d = validation_data[(u8_t)e->type];
        check(e->num_inputs >= d.min_inputs && e->num_inputs <= d.max_inputs, "Input count mismatch, expected {} <= {} <= {}", d.min_inputs, e->num_inputs, d.max_inputs);
        for (u32_t i = 0U, n = e->num_inputs; i < n; ++i) {
          child_index = i;
          auto & in = e->input(i);
          if (!in.expr) {
            check(is_branch(in.expr->type, i), "Child expression is null");
            continue;
          }
          cur_expr = in.expr;
          check(cur_expr->parent == e, "Parent mismatch");
          check(cur_expr->parent_pos == i, "Parent position mismatch");
          visit(cur_expr);
        }
      }

      void execute(expr * J_NOT_NULL e) {
        check(e->type == expr_type::fn_body, "Expected root to be fn-body");
        visit(e);
        for (u8_t i = 0; i < num_expr_types_v; ++i) {
          check(counts[i] == fn->counts[i], "Expr count mismatch. Counted {} of {}, fn had {}.", counts[i], (expr_type)i, fn->counts[i]);
        }
      }
    };

    void basic_validate(air_pass_context & ctx) {
      basic_validation_visitor v{
        ctx.ctx,
        ctx.fn};
      v.execute(ctx.fn->body);
    }
  }

  const air_pass basic_validation_pass{
    basic_validate,
    "basic-validation",
    "Basic validation",
  };
}
