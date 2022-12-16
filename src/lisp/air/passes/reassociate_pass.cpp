#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/accessors.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    enum class reassoc_type : u8_t {
      none,
      full,
      tail,
      head,
    };

    enum class reassoc_preference : u8_t {
      none,
      immovable,
      head,
      tail,
      special,
    };

    struct J_TYPE_HIDDEN reassoc_rec final {
      reassoc_type type;
      reassoc_preference pref;
    };

#define J_REASSOC_NONE(EXPR, PREF) [(u8_t)expr_type::EXPR] = { reassoc_type::none, reassoc_preference::PREF }
#define J_REASSOC_FULL(EXPR, PREF) [(u8_t)expr_type::EXPR] = { reassoc_type::full, reassoc_preference::PREF }
#define J_REASSOC_TAIL(EXPR, PREF) [(u8_t)expr_type::EXPR] = { reassoc_type::tail, reassoc_preference::PREF }
#define J_REASSOC_HEAD(EXPR, PREF) [(u8_t)expr_type::EXPR] = { reassoc_type::head, reassoc_preference::PREF }

    constexpr inline reassoc_rec reassoc_data[num_expr_types_v] {
      // Nops are immovable, as they are only used as placeholders.
      J_REASSOC_HEAD(nop, immovable),

      J_REASSOC_HEAD(fn_body, immovable),
      // Args are immovable until reg alloc behaves.
      J_REASSOC_NONE(fn_arg, immovable),
      J_REASSOC_NONE(fn_rest_arg, immovable),
      J_REASSOC_NONE(fn_sc_arg, immovable),

      // Special case, weight computed by writes.
      J_REASSOC_NONE(lex_rd, special),
      // Prefer bottom, lex_wr might use a register.
      J_REASSOC_NONE(lex_wr, tail),
      // Prefer bottom, while memory accesses should happen early, reading memory uses a register.
      J_REASSOC_NONE(lex_rd_bound, tail),
      // Prefer top, memory accesses should happen early and writes often free registers.
      J_REASSOC_NONE(lex_wr_bound, head),
      /// \todo register pressure sensitivity (as let temporarily takes some registers)
      J_REASSOC_HEAD(let, none),

      // Prefer bottom, while memory accesses should happen early, reading memory uses a register.
      J_REASSOC_NONE(sym_val_rd, tail),
      // Prefer top, memory accesses should happen early and writes often free registers.
      J_REASSOC_NONE(sym_val_wr, head),

      // Prefer tail, as constants should be near their use. Otherwise they might need a register.
      J_REASSOC_NONE(ld_const, tail),

      J_REASSOC_NONE(ineg, none),
      // Prefer top, this usually combined multiple parameters.
      J_REASSOC_FULL(iadd, head),
      // Prefer top, this usually combined multiple parameters.
      J_REASSOC_TAIL(isub, head),
      // Prefer top, this usually combined multiple parameters.
      J_REASSOC_FULL(imul, head),
      // Prefer top, this usually combined multiple parameters.
      J_REASSOC_TAIL(idiv, head),

      // Prefer top for asserts, to make it easier to remove redundant checks in some later passes maybe
      J_REASSOC_NONE(type_assert, head),
      J_REASSOC_NONE(dyn_type_assert, head),

      J_REASSOC_NONE(b_if, head),
      J_REASSOC_NONE(do_until, none),

      J_REASSOC_NONE(lor, none),
      J_REASSOC_NONE(land, none),
      J_REASSOC_NONE(lnot, tail),

      J_REASSOC_NONE(to_bool, tail),

      J_REASSOC_FULL(eq, head),
      J_REASSOC_FULL(neq, head),

      J_REASSOC_NONE(tag, none),
      J_REASSOC_NONE(untag, none),

      J_REASSOC_NONE(as_range, none),
      J_REASSOC_NONE(as_vec, none),

      J_REASSOC_HEAD(progn, none),
      J_REASSOC_TAIL(prog1, none),
      J_REASSOC_NONE(call, tail),
      J_REASSOC_NONE(full_call, tail),

      // Prefer top, this usually combined multiple parameters.
      J_REASSOC_NONE(vec, head),
      // Prefer top, this usually combined multiple parameters.
      J_REASSOC_NONE(vec_build, head),
      // Should maybe prefer top, to make room for user code. But alloc is function call.
      J_REASSOC_NONE(make_vec, tail),
      // Should maybe prefer top, to make room for user code. But alloc is function call.
      J_REASSOC_NONE(range_copy, tail),
      // Prefer top, this usually combined multiple parameters.
      // OTOH, memory alloc is currently a function call, so until that gets immovable, this will trash registers.
      J_REASSOC_NONE(vec_append, head),

      // Preference computed solely by inputs.
      J_REASSOC_NONE(range_length, none),
      // Prefer top, memory accesses should happen early.
      J_REASSOC_NONE(range_get, head),
      // Prefer top, memory accesses should happen early.
      J_REASSOC_NONE(range_set, head),

      // Ideally would prefer top, to avoid mNLX conflicts. Additionally, the parent parameter might free a register.
      // OTOH, memory alloc is currently a function call, so until that gets immovable, this will trash registers.
      J_REASSOC_NONE(act_rec_alloc, head),
      // Prefer top, memory writes should happen early.
      J_REASSOC_NONE(act_rec_wr, head),
      // Prefer top, memory reads should happen early.
      J_REASSOC_NONE(act_rec_rd, head),
      // Prefer top, memory reads should happen early.
      J_REASSOC_NONE(act_rec_parent, head),

      J_REASSOC_NONE(closure, none),
    };

    enum class swap_result : u8_t {
      ok = 0U,

      a_immovable,
      b_immovable,

      wr_wr,
      wr_rd,
      wr_nlx,
      wr_fx,

      lex_wr_wr,
      lex_wr_rd,
      lex_rd_wr,

      rd_wr,
      rd_fx,

      nlx_wr,
      nlx_fx,
      nlx_nlx,

      fx_fx,
      fx_rd,
      fx_wr,
      fx_nlx
    };
    constexpr const char * color = "#F0F098";

    struct J_TYPE_HIDDEN reassociate_visitor final {
      debug_info_map * dbg;

      void debug_swap(exprs::expr * J_NOT_NULL J_RESTRICT a, exprs::expr * J_NOT_NULL J_RESTRICT b, swap_result result, i32_t index) const noexcept {
        if (result == swap_result::a_immovable) {
          dbg->add_oneline(a, "Immovable", color);
          return;
        }

        strings::string other = strings::format("{}@{}", b->type, index);

        const char * type = nullptr;
        switch (result) {
        case swap_result::ok:
        case swap_result::a_immovable:
          J_UNREACHABLE();
        case swap_result::b_immovable:
          dbg->add_oneline(a, other + " is immovable", color);
          return;

        case swap_result::wr_wr:
          type = "WR-WR";
          break;
        case swap_result::wr_rd:
          type = "WR-RD";
          break;
        case swap_result::wr_nlx:
          type = "WR-NLX";
          break;
        case swap_result::wr_fx:
          type = "WR-FX";
          break;

        case swap_result::lex_wr_wr:
          type = "Lex WR-WR";
          break;
        case swap_result::lex_wr_rd:
          type = "Lex WR-RD";
          break;
        case swap_result::lex_rd_wr:
          type = "Lex RD-WR";
          break;

        case swap_result::rd_wr:
          type = "RD-WR";
          break;
        case swap_result::rd_fx:
          type = "RD-FX";
          break;

        case swap_result::nlx_wr:
          type = "NLX-WR";
          break;
        case swap_result::nlx_fx:
          type = "NLX-FX";
          break;
        case swap_result::nlx_nlx:
          type = "NLX-NLX";
          break;

        case swap_result::fx_fx:
          type = "FX-FX";
          break;
        case swap_result::fx_rd:
          type = "FX-RD";
          break;
        case swap_result::fx_wr:
          type = "FX-WR";
          break;
        case swap_result::fx_nlx:
          type = "FX-NLX";
          break;
        }
        dbg->add(a, type, static_cast<strings::string &&>(other), color);
      }

      J_INLINE_GETTER swap_result check_swap(exprs::expr * J_NOT_NULL J_RESTRICT a, exprs::expr * J_NOT_NULL J_RESTRICT b) const noexcept {
        if (reassoc_data[(u8_t)a->type].pref == reassoc_preference::immovable) {
          return swap_result::a_immovable;
        }
        if (reassoc_data[(u8_t)b->type].pref == reassoc_preference::immovable) {
          return swap_result::b_immovable;
        }
        if (a->writes_lex) {
          if (b->writes_lex) {
            return swap_result::lex_wr_wr;
          }
          if (b->reads_lex) {
            return swap_result::lex_wr_rd;
          }
        }
        if (a->reads_lex) {
          if (b->writes_lex) {
            return swap_result::lex_rd_wr;
          }
        }
        if (a->writes_mem) {
          if (b->writes_mem) {
            return swap_result::wr_wr;
          }
          if (b->reads_mem) {
            return swap_result::wr_rd;
          }
          if (b->may_nlx || b->movable_nlx) {
            return swap_result::wr_nlx;
          }
          if (b->has_side_effects) {
            return swap_result::wr_fx;
          }
        }
        if (a->reads_mem) {
          if (b->has_side_effects) {
            return swap_result::rd_fx;
          }
          if (b->writes_mem) {
            return swap_result::rd_wr;
          }
        }
        if (a->may_nlx || a->movable_nlx) {
          if (b->writes_mem) {
            return swap_result::nlx_wr;
          }
          if (b->has_side_effects) {
            return swap_result::nlx_fx;
          }
          if (b->may_nlx || (a->may_nlx && b->movable_nlx)) {
            return swap_result::nlx_nlx;
          }
        }
        if (a->has_side_effects) {
          if (b->has_side_effects) {
            return swap_result::fx_fx;
          }
          if (b->reads_mem) {
            return swap_result::fx_rd;
          }
          if (b->writes_mem) {
            return swap_result::fx_wr;
          }
          if (b->may_nlx || b->movable_nlx) {
            return swap_result::fx_nlx;
          }
        }
        return swap_result::ok;
      }

      void move_to(exprs::input * J_NOT_NULL inputs, i32_t from, i32_t to, i32_t * J_NOT_NULL weights) const noexcept {
        J_ASSUME(from > to);
        i32_t weight = weights[from];
        ::j::memmove(weights + to + 1, weights + to, (from - to) * sizeof(i32_t));
        weights[to] = weight;

        exprs::expr * expr_from = inputs[from].expr;
        u8_t parent_pos = inputs[to].expr->parent_pos;
        ::j::memmove(inputs + to + 1, inputs + to, (from - to) * sizeof(exprs::input));
        inputs[to].expr = expr_from;

        for (exprs::input * it = inputs + to, * const end = inputs + from + 1; it != end; ++it, ++parent_pos) {
          it->expr->parent_pos = parent_pos;
        }
      }

      void debug_weight(const exprs::expr * J_NOT_NULL e, i32_t weight, reassoc_preference pref) {
        if (!dbg) {
          return;
        }
        if (weight == I32_MIN || pref == reassoc_preference::immovable) {
          dbg->add(e, "Weight", "Immovable", "#C8C880");
        } else {
          dbg->add(e, "Weight", weight, "#A0A090");
        }
      }

      i32_t visit(exprs::expr * J_NOT_NULL e) noexcept {
        const reassoc_rec & d = reassoc_data[(u8_t)e->type];
        auto pref = d.pref;
        reassoc_type type = d.type;
        i32_t weight = 0;
        switch (pref) {
        case reassoc_preference::special:
          switch (e->type) {
          case expr_type::lex_rd: {
            exprs::lex_rd_accessor rd(*e);
            for (auto & wr : rd.reaching_writes()) {
              switch (wr->input(0).expr->type) {
              case expr_type::fn_arg:
                weight -= 3;
                break;
              case expr_type::full_call:
              case expr_type::make_vec:
              case expr_type::range_copy:
                weight -= 4;
                break;
              case expr_type::fn_rest_arg:
                weight -= 7;
                break;
              case expr_type::ld_const:
                --weight;
                break;
              default:
                weight -=2;
                break;
              }
            }
            break;
          }
          default: J_FAIL("Special case was not handled.");
          }
          break;
        case reassoc_preference::head:
          weight = -2;
          break;
        case reassoc_preference::tail:
          weight = (e->type == expr_type::lex_wr || e->type == expr_type::sym_val_rd) ? 8 : 2;
          break;
        case reassoc_preference::none:
        case reassoc_preference::immovable:
          break;
        }

        auto inputs = e->inputs();
        if (e->type == expr_type::closure) {
          visit(inputs[1].expr);
          i32_t weight = visit(inputs[0].expr);
          if (dbg) {
            dbg->add_oneline(e, "Closure special case", "#80F0FF");
            debug_weight(e, weight, pref);
          }
          return weight;
        } else if (type == reassoc_type::none || inputs.size() <= 1U || (inputs.size() == 2U && (type == reassoc_type::head || type == reassoc_type::tail))) {
          for (auto & in : inputs) {
            i32_t in_weight = visit(in.expr);
            weight = (weight == I32_MIN || in_weight == I32_MIN) ? I32_MIN : weight + in_weight;
          }
          if (dbg) {
            dbg->add_oneline(e, type == reassoc_type::none ? "Expr not sortable" : "Nothing to sort");
            debug_weight(e, weight, pref);
          }
          return pref == reassoc_preference::immovable ? I32_MIN : weight;
        } else if (type == reassoc_type::head) {
          weight += visit(inputs.pop_back().expr);
        } else if (type == reassoc_type::tail) {
          weight += visit(inputs.pop_front().expr);
        }

        i32_t in_weights[inputs.size()];

        for (i32_t i = 0, end = (i32_t)inputs.size(); i < end; ++i) {
          auto in = inputs[i].expr;
          auto in_weight = visit(in);
          in_weights[i] = in_weight;

          if (in_weight == I32_MIN) {
            pref = reassoc_preference::immovable;
            continue;
          }
          weight += in_weight;

          i32_t ins = i;
          for(; ins > 0; --ins) {
            if (in_weights[ins - 1] <= in_weight) {
              break;
            }
            auto result = check_swap(in, inputs[ins - 1].expr);
            if (result != swap_result::ok) {
              if (dbg) {
                debug_swap(in, inputs[ins - 1].expr, result, ins - i - 1);
              }
              break;
            }
          }
          if (ins != i) {
            if (dbg) {
              dbg->add(in, "Moved", ins - i, "#C8F0C8");
            }
            move_to(inputs.begin(), i, ins, in_weights);
          }
        }
        debug_weight(e, weight, pref);
        return pref == reassoc_preference::immovable ? I32_MIN : weight;
      }
    };

    void apply_reassociate_pass(air_pass_context & ctx) noexcept {
      reassociate_visitor v{ctx.debug_info};
      v.visit(ctx.fn->body);
    }
  }

  const air_pass reassociate_pass{apply_reassociate_pass, "reassociate", "Reorder expressions"};
}
