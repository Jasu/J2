#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/exprs/expr_data.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/functions/air_function.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN barrier_flags {
      bool has_side_effects = false;
      bool writes_mem = false;
      bool reads_mem = false;
      bool writes_lex = false;
      bool reads_lex = false;
      bool may_nlx = false;
      bool movable_nlx = false;

      static barrier_flags from_expr(const exprs::expr * J_NOT_NULL e) noexcept {
        auto & f = exprs::expr_data_v[e->type].flags;
        return {
          .has_side_effects = f.has(exprs::expr_flag::has_side_effects),
          .writes_mem       = f.has(exprs::expr_flag::writes_memory),
          .reads_mem        = f.has(exprs::expr_flag::reads_memory),
          .writes_lex       = f.has(exprs::expr_flag::writes_lex),
          .reads_lex        = f.has(exprs::expr_flag::reads_lex),
          .may_nlx          = f.has(exprs::expr_flag::may_nlx),
          .movable_nlx      = f.has(exprs::expr_flag::movable_nlx),
        };
      }

      explicit operator bool() const noexcept {
        return reads_lex || writes_lex || writes_mem || reads_mem || has_side_effects || may_nlx;
      }

      void to_expr(exprs::expr * J_NOT_NULL e) const noexcept {
        e->has_side_effects = has_side_effects;
        e->writes_mem       = writes_mem;
        e->reads_mem        = reads_mem;
        e->writes_lex       = writes_lex;
        e->reads_lex        = reads_lex;
        e->may_nlx          = may_nlx;
        e->movable_nlx      = movable_nlx;
      }

      barrier_flags & operator|=(const barrier_flags & rhs) noexcept {
        has_side_effects |= rhs.has_side_effects;
        writes_mem       |= rhs.writes_mem;
        reads_mem        |= rhs.reads_mem;
        writes_lex       |= rhs.writes_lex;
        reads_lex        |= rhs.reads_lex;
        may_nlx          |= rhs.may_nlx;
        movable_nlx      |= rhs.movable_nlx;
        return *this;
      }
    };

    struct propagate_barriers_visitor final {
      debug_info_map * dbg;

      void debug_barrier_flags(const exprs::expr * J_NOT_NULL e, const barrier_flags & flags, const char * J_NOT_NULL key) noexcept {
        strings::string value;
        const char * color = "#99F099";
        if (flags.writes_lex) {
          if (flags.reads_lex) {
            value += value ? ",lex-rw" : "lex-rw";
          } else {
            value = "lex-wr";
          }
          color = "#F0F099";
        } else if (flags.reads_lex) {
          value += value ? ",lex-rd" : "lex-rd";
        }

        if (flags.writes_mem) {
          if (flags.reads_mem) {
            value += value ? ",mem-rw" : "mem-rw";
          } else {
            value += value ? ",mem-wr" : "mem-wr";
          }
          color = "#FFFF40";
        } else if (flags.reads_mem) {
          value += value ? ",mem-rd" : "mem-rd";
          color = "#F9F079";
        }
        if (flags.may_nlx) {
          value += value ? ",nlx" : "nlx";
          color = "#FF68FF";
        } else if (flags.movable_nlx) {
          value += value ? ",mov-nlx" : "mov-nlx";
          color = "#FF68FF";
        }
        if (flags.has_side_effects) {
          value += value ? ",side" : "side";
          color = "#FF6868";
        }
        dbg->add(e, key, static_cast<strings::string &&>(value), color);
      }

      barrier_flags propagate_barriers(exprs::expr * J_NOT_NULL e) noexcept {
        barrier_flags result = barrier_flags::from_expr(e);
        if (dbg && result) {
          debug_barrier_flags(e, result, "Own");
        }
        if (e->type == expr_type::closure) {
          propagate_barriers(e->input(1).expr);
          result |= propagate_barriers(e->input(0).expr);
        } else {
          for (auto & in : e->inputs()) {
            result |= propagate_barriers(in.expr);
          }
        }
        result.to_expr(e);
        if (dbg && result) {
          debug_barrier_flags(e, result, "Sum");
        }
        return result;
      }
    };

    void apply_propagate_barriers(air_pass_context & ctx) {
      propagate_barriers_visitor v{ctx.debug_info};
      v.propagate_barriers(ctx.fn->body);
    }
  }

  const air_pass propagate_barriers_pass{apply_propagate_barriers, "propagate-barriers", "Propagate barriers", true};
}
