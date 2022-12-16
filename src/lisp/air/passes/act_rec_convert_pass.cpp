#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "logging/global.hpp"
#include "lisp/air/functions/air_closure.hpp"
#include "lisp/air/exprs/expr_builder.hpp"
#include "lisp/air/exprs/accessors.hpp"
#include "lisp/air/functions/activation_record.hpp"
#include "lisp/values/lisp_imms.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN act_rec_convert_visitor final {
      J_RETURNS_NONNULL exprs::expr * create_rd_act_rec(u32_t index) {
        activation_record * var_rec = nullptr;
        for (auto & rec : b.fn.activation_records) {
          if (rec.index == index) {
            var_rec = &rec;
          }
        }
        J_ASSERT_NOT_NULL(var_rec);
        auto res = b.create(expr_type::lex_rd, &var_rec->act_rec_var);
        res->result.reps.set_mask(rep_untagged);
        res->result.reps.set_rep(rep_untagged);
        res->result.types = imm_act_rec;
        return res;
      }

      exprs::expr_builder b;
      air_closure * cur_closure = nullptr;

      bool add_alloc_fn(exprs::expr * J_NOT_NULL e, activation_record & rec) {
        auto in = e->inputs_begin();
        air_var * vars[rec.num_vars];
        exprs::expr * exprs[rec.num_vars];
        u32_t num_vars = 0U;
        for (; in != e->inputs_end(); ++in) {
          if (in->expr->type != expr_type::lex_wr && in->expr->type != expr_type::lex_wr_bound) {
            break;
          }
          auto type = in->expr->input(0).expr->type;
          if (type != expr_type::fn_arg && type != expr_type::fn_sc_arg) {
            break;
          }

          auto acc = exprs::lex_wr_accessor(*in->expr);
          if (acc.var()->activation_record_index == rec.index) {
            vars[num_vars] = acc.var();
            exprs[num_vars++] = in->expr;
          }
        }
        if (!num_vars) {
          // return false;
        }
        exprs::expr * parent = rec.parent
          ? create_rd_act_rec(rec.parent->index)
          : b.create(expr_type::ld_const, copy_metadata(e->metadata()), lisp_nil{});

        exprs::expr * progn_exprs[num_vars + 2U];
        progn_exprs[0] = b.create(expr_type::lex_wr,
                                  copy_metadata(e->metadata()),
                                  &rec.act_rec_var,
                                  b.create(expr_type::act_rec_alloc,
                                           copy_metadata(e->metadata()),
                                           (u32_t)rec.index,
                                           parent));
        for (u32_t i = 0U; i < num_vars; ++i) {
          auto wr = progn_exprs[i + 1] = b.create(
              expr_type::act_rec_wr,
              copy_metadata(e->metadata()),
              vars[i],
              create_rd_act_rec(rec.index),
              b.create(expr_type::lex_rd, vars[i]));
          wr->input(1).expr->result = exprs[i]->result;
          wr->input(1).expr->result.reps.set_rep(rep_tagged);
          wr->result = exprs[i]->result;
        }
        progn_exprs[num_vars + 1U] = in->expr;
        auto progn = b.create_late(
          in->expr->parent, in->expr->parent_pos,
          expr_type::progn,
          copy_metadata(e->metadata()),
          span(progn_exprs, num_vars + 2));
        progn->result = progn_exprs[num_vars + 1]->result;
        visit(progn_exprs[num_vars+1U]);
        for (++in; in != e->inputs_end(); ++in) {
          visit(in->expr);
        }
        return true;
      }

      void add_alloc(exprs::expr * J_NOT_NULL e, activation_record & rec) {
        for (auto & in : e->inputs()) {
          if (in.expr->type != expr_type::lex_wr && in.expr->type != expr_type::lex_wr_bound) {
            continue;
          }
          auto acc = exprs::lex_wr_accessor(*in.expr);
          if (acc.var()->activation_record_index != rec.index) {
            continue;
          }
          exprs::expr * parent = rec.parent
            ? create_rd_act_rec(rec.parent->index)
            : b.create(expr_type::ld_const,
                       copy_metadata(e->metadata()),
                       lisp_nil{});

          exprs::expr * progn_exprs[]{
            b.create(expr_type::lex_wr,
                     copy_metadata(e->metadata()),
                     &rec.act_rec_var,
                     b.create(expr_type::act_rec_alloc,
                              copy_metadata(e->metadata()),
                              (u32_t)rec.index,
                              parent)),
              in.expr
              };
          auto progn = b.create_late(
            in.expr->parent, in.expr->parent_pos,
            expr_type::progn,
            copy_metadata(e->metadata()),
            progn_exprs);
          progn->result = progn_exprs[1]->result;
          return;
        }
        J_DEBUG("No write to an activated variable found: ", *e);
        J_FAIL("No write to an activated variable found.");
      }

      void visit(exprs::expr * J_NOT_NULL e) noexcept {
        switch (e->type) {
        case expr_type::fn_body:
          if (cur_closure && cur_closure->activation_record) {
            u32_t act_rec_count = 0U;
            for (auto rec = cur_closure->activation_record; rec; rec = rec->parent) {
              ++act_rec_count;
            }
            air_var * rec_var = &cur_closure->activation_record->act_rec_var;
            auto wr_act_rec =
              b.create(expr_type::lex_wr,
                       copy_metadata(e->metadata()),
                       rec_var,
                       b.create(expr_type::fn_sc_arg, copy_metadata(e->metadata())));

            exprs::expr * progn_exprs[act_rec_count + 1];
            progn_exprs[0] = wr_act_rec;
            u32_t i = 1U;
            for (auto rec = cur_closure->activation_record->parent;
                 rec;
                 rec = rec->parent, ++i) {
              progn_exprs[i] = b.create(expr_type::lex_wr,
                                        copy_metadata(e->metadata()),
                                        &rec->act_rec_var,
                                        b.create(expr_type::act_rec_parent,
                                                 copy_metadata(e->metadata()),
                                                 b.create(expr_type::lex_rd,
                                                          copy_metadata(e->metadata()),
                                                          rec_var)));
              rec_var = &rec->act_rec_var;
            }
            progn_exprs[i++] = e->in_expr(0);
            b.create_late(
              e, 0,
              expr_type::progn,
              copy_metadata(e->metadata()),
              span(progn_exprs, i));
          }
          for (auto & rec : b.fn.activation_records) {
            if (rec.lowest_scope == e) {
              if (add_alloc_fn(e, rec)) {
                return;
              }
              add_alloc(e, rec);
              break;
            }
          }
          break;
        case expr_type::let:
          for (auto & rec : b.fn.activation_records) {
            if (rec.lowest_scope == e) {
              add_alloc(e, rec);
              break;
            }
          }
          break;

        case expr_type::lex_wr:
        case expr_type::lex_wr_bound: {
          auto acc = exprs::lex_wr_accessor(*e);
          if (acc.var()->is_bound) {
            b.create_late(
              e->parent, e->parent_pos,
              expr_type::act_rec_wr,
              copy_metadata(e->metadata()),
              acc.var(),
              create_rd_act_rec(acc.var()->activation_record_index),
              e->input(0).expr
            );
          }
          break;
        }

        case expr_type::lex_rd:
        case expr_type::lex_rd_bound: {
          auto acc = exprs::lex_rd_accessor(*e);
          if (acc.var()->is_bound) {
            auto rd = b.create_late(
              e->parent, e->parent_pos,
              expr_type::act_rec_rd,
              copy_metadata(e->metadata()),
              acc.var(),
              create_rd_act_rec(acc.var()->activation_record_index));
            rd->result.types = e->result.types;
          }
          return;
        }
        case expr_type::closure: {
          auto prev_closure = cur_closure;
          cur_closure = *reinterpret_cast<air_closure**>(e->static_begin());
          b.attach_to(e, 0, create_rd_act_rec(cur_closure->activation_record->index));
          visit(e->input(1).expr);
          cur_closure = prev_closure;
          return;
        }
        default:
          break;
        }

        for (auto & in : e->inputs()) {
          visit(in.expr);
        }
      }
    };

    void apply_act_rec_convert_pass(air_pass_context & ctx) noexcept {
      const u32_t num_recs = ctx.fn->activation_records.size();
      if (!num_recs) {
        return;
      }
      act_rec_convert_visitor v{{ctx.pool, *ctx.fn}};
      v.visit(ctx.fn->body);
    }
  }

  const air_pass act_rec_convert_pass{apply_act_rec_convert_pass, "act-rec-convert", "Convert activation records", true};
}
