#include "lisp/air/passes/air_passes.hpp"
#include "algo/quicksort.hpp"
#include "bits/bitset.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/functions/air_closure.hpp"
#include "lisp/air/functions/activation_record.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    struct J_TYPE_HIDDEN scope_key final {
      exprs::expr * scope_expr;
      air_function * fn;
      air_closure * closure;
      u32_t max_depth = 0U;
      u32_t max_binding_depth = 0U;

      J_INLINE_GETTER static exprs::expr * get_key(const scope_key & k) noexcept {
        return k.scope_expr;
      }
    };

    using keys_t = span<scope_key>;

    struct J_TYPE_HIDDEN act_rec_compute_visitor final {
      j::mem::bump_pool & pool;
      air_function * fn = nullptr;
      bits::bitset bound_keys;
      u8_t index = 1U;
      packages::pkg & pkg;

      activation_record * make_rec(keys_t keys, u32_t depth, activation_record * parent, exprs::expr * J_NOT_NULL lowest_scope, bool add_all = false) {
        u32_t num_bound = bound_keys.size();
        if (!num_bound) {
          return parent;
        }
        u32_t num_closures = 0U;

        for (auto & closure : keys) {
          if ((add_all || closure.closure->max_binding_depth <= depth) && !closure.closure->activation_record) {
            ++num_closures;
          }
        }

        pool.align(8);
        activation_record * rec = &pool.emplace_with_padding<activation_record>(
          (num_closures + num_bound) * sizeof(void*),
          index++,
          num_closures,
          num_bound,
          lowest_scope,
          air_var{pkg.gensym(), fn->num_lex_vars++},
          parent);
        auto it = rec->closures_begin();
        u32_t i = 0U;
        for (auto & closure : keys) {
          if ((add_all || closure.closure->max_binding_depth <= depth) && !closure.closure->activation_record) {
            closure.closure->activation_record = rec;
            closure.closure->act_rec_closure_index = i++;
            *it++ = closure.closure;
          }
        }
        J_ASSERT(it == rec->closures().end());

        auto key_it = rec->vars_begin();
        for (;;) {
          auto idx = bound_keys.find_and_clear_first_set();
          if (idx == bound_keys.npos) {
            break;
          }
          air_var * var = fn->lex_vars[idx];
          var->activation_record_index = rec->index;
          *key_it++ = var;
        }

        fn->activation_records.push_back(rec);
        bound_keys.clear();
        return rec;
      }

      bool requires_split(keys_t keys, u32_t level) const noexcept {
        exprs::expr * cur = nullptr;
        for (auto & key : keys) {
          if (key.scope_expr) {
            u8_t prev_loop_depth = level ? key.closure->scopes()[level - 1].loop_depth : 0;
            if (key.closure->scopes()[level].loop_depth != prev_loop_depth) {
              return true;
            } else if (!cur) {
              cur = key.scope_expr;
            } else if (cur != key.scope_expr) {
              return true;
            }
          }
        }
        return false;
      }

      void add_bound_keys(keys_t keys, u32_t depth) noexcept {
        for (auto & key : keys) {
          if (key.scope_expr) {
            for (auto & b : key.closure->scopes()[depth]) {
              if (!b.activation_record_index) {
                bound_keys.set(b.index);
              }
            }
          }
        }
      }

      bool set_key_levels(keys_t keys, u32_t depth) noexcept {
        bool had = false;
        for (auto & key : keys) {
          if (key.max_depth > depth) {
            key.scope_expr = key.closure->scopes()[depth].expr;
            had = true;
            continue;
          }
          key.scope_expr = nullptr;
        }
        return had;
      }


      bool recurse(keys_t keys, u32_t depth, exprs::expr * J_NOT_NULL lowest_scope, activation_record * parent = nullptr) {
        if (!requires_split(keys, depth)) {
          add_bound_keys(keys, depth);
          bool result = set_key_levels(keys, depth + 1U) && recurse(keys, depth + 1U, lowest_scope);
          if (!result) {
            if (depth == 0) {
              make_rec(keys, depth, parent, lowest_scope, true);
            }
            return false;
          }
        }

        algo::quicksort(keys, scope_key::get_key);

        auto begin = keys.begin(), it = begin, end = keys.end();
        for (; it != end && !it->scope_expr; ++it) { }
        parent = make_rec({it, end}, depth - 1, parent, lowest_scope);

        begin = it;
        for (exprs::expr * cur = nullptr; it != end; ++it) {
          if (!cur) {
            cur = it->scope_expr;
          }
          if (cur == it->scope_expr) {
            continue;
          }
          cur = it->scope_expr;

          keys_t subrange{begin, it};
          add_bound_keys(subrange, depth);
          if (!set_key_levels(subrange, depth + 1U) || !recurse(subrange, depth + 1U, begin->closure->scopes()[depth].expr, parent)) {
            make_rec(subrange, depth, parent, begin->closure->scopes()[depth].expr);
          }
          begin = it;
        }
        if (begin != end) {
          keys_t subrange{begin, end};
          add_bound_keys(subrange, depth);
          if (!set_key_levels(subrange, depth + 1U) || !recurse(subrange, depth + 1U, begin->closure->scopes()[depth].expr, parent)) {
            make_rec(subrange, depth, parent,  begin->closure->scopes()[depth].expr);
          }
        }
        return true;
      }
    };

    void apply_act_rec_compute_pass(air_pass_context & ctx) noexcept {
      auto fn = ctx.fn;
      if (!fn->num_closures) {
        return;
      }

      scope_key keys[fn->num_closures];
      u32_t i = 0U;
      for (auto s = fn->first_closure; s; s = s->next_closure) {
        keys[i++] = {
          .scope_expr = s->scopes()[0].expr,
          .closure = s,
          .max_depth = s->scope_depth,
          .max_binding_depth = s->max_binding_depth + 1U,
        };
      }
      J_ASSUME(i == fn->num_closures);
      act_rec_compute_visitor v{
        .pool = *ctx.pool,
        .fn = ctx.fn,
        .bound_keys{fn->num_lex_vars},
        .pkg = ctx.ctx.pkg,
      };
      v.recurse(keys_t(keys, i), 0U, fn->body);

      // J_DEBUG("{#bold}Activation records:");
      // for (auto & rec : fn->activation_records) {
      //   J_DEBUG("\n{}\n", rec);
      // }
    }
  }

  const air_pass act_rec_compute_pass{apply_act_rec_compute_pass, "act-rec-compute", "Compute activation records"};
}
