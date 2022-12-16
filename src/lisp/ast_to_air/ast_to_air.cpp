#include "lisp/ast_to_air/ast_to_air.hpp"
#include "lisp/ast_to_air/parse_utils.hpp"
#include "lisp/compilation/compilation_errors.hpp"
#include "lisp/compilation/compile_defun.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/compilation/macroexpand.hpp"
#include "lisp/air/analysis/lexical_scope.hpp"
#include "lisp/air/functions/closure_manager.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/air_package.hpp"
#include "lisp/air/passes/air_pass.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/air/exprs/expr_builder.hpp"
#include "exceptions/assert.hpp"

namespace j::lisp::air::inline functions {
  air_function::air_function() noexcept {
  }
}
namespace j::lisp::ast_to_air {
  namespace {
    using e = air::expr_type;
    using namespace j::lisp::compilation;


    struct J_TYPE_HIDDEN ast_visitor final {
      air::exprs::expr_builder builder;

      compilation::context & ctx;
      air::analysis::lexical_scope_manager scope;
      air::closure_manager closures;
      id current_form = id::none;

      ast_visitor(compilation::context & ctx, air::air_function * J_NOT_NULL afn) noexcept
        : builder{ .pool = &ctx.temp_pool, *afn },
          ctx(ctx),
          closures{ctx.temp_pool, scope, afn}
      { }

      air::air_vars_t alloc_vars(u16_t size) {
        if (!size) {
          return {};
        }
        ctx.temp_pool.align(alignof(air::air_var));
        return {
          (air::air_var*)ctx.temp_pool.allocate(sizeof(air::air_var) * size),
          size
        };
      }

      metadata_init cur_loc;

      template<typename... Args>
      air::exprs::expr * create_expr(air::expr_type e, Args && ... args) {
        if (ctx.source_location()) {
          return builder.create(e, cur_loc, static_cast<Args &&>(args)...);
        } else {
          return builder.create(e, static_cast<Args &&>(args)...);
        }
      }

      [[noreturn]] void throw_symbol_error(id_resolution_error err, id sym) {
        compilation::throw_compilation_error(ctx.source_location(), current_form, err, sym);
      }

      void check_arg_count(imms_t body, u32_t expected) {
        if (body.size() != expected) {
          throw_compilation_error(ctx.source_location(), current_form, arg_count_mismatch, expected, body.size());
        }
      }

      void check_arg_min_count(imms_t body, u32_t expected) {
        if (body.size() < expected) {
          throw_compilation_error(ctx.source_location(), current_form, too_few_args, expected, body.size());
        }
      }

      air::exprs::expr * parse_expr_3(air::expr_type t, imms_t body, bool is_result_used) {
        check_arg_count(body, 3);
        if (!is_result_used) {
          return parse_progn(body, false);
        }
        return create_expr(t, parse_form(body[0], true), parse_form(body[1], true), parse_form(body[2], true));
      }

      template<typename... Extra>
        air::exprs::expr * parse_n_ary_expr(air::expr_type t, imms_t body, bool is_result_used, Extra && ... extra) {
        if (!is_result_used) {
          return parse_progn(body, false);
        }
        const u32_t sz = body.size();
        air::exprs::expr * exprs[sz];
        for (u32_t i = 0U; i < sz; ++i) {
          exprs[i] = parse_form(body[i], true);
        }
        return create_expr(t, span(exprs, sz), static_cast<Extra &&>(extra)...);
      }

      air::exprs::expr * parse_unary_expr(air::expr_type t, imms_t body, bool is_result_used) {
        check_arg_count(body, 1);
        auto in = parse_form(body[0], is_result_used);
        return is_result_used ? create_expr(t, in) : in;
      }

      air::exprs::expr * parse_binary_expr(air::expr_type t, imms_t body, bool is_result_used) {
        check_arg_count(body, 2);
        if (!is_result_used) {
          return parse_progn(body, false);
        }
        return create_expr(t, parse_form(body[0], true), parse_form(body[1], true));
      }

      air::exprs::expr * parse_repeated_binary_expr(air::expr_type t, imms_t body, bool is_result_used) {
        if (body.size() == 1) {
          return parse_form(body[0], is_result_used);
        }
        check_arg_min_count(body, 2);
        if (!is_result_used) {
          return parse_progn(body, false);
        }
        return create_expr(t, parse_repeated_binary_expr(t, body.without_suffix(1), true), parse_form(body.back(), true));
      }

      air::exprs::expr * parse_closure(imms_t body, bool is_result_used) {
        if (!is_result_used) {
          return nullptr;
        }
        check_arg_min_count(body, 2);
        auto closure_func_info = compilation::parse_func_info(ctx, body.pop_front().as_vec_ref()->value());
        auto sym = ctx.alloc_closure_symbol(builder.fn.num_closures, closure_func_info);
        auto info = closures.enter_closure(closure_func_info, sym);
        auto fn_body = parse_fn_body(body, closure_func_info, true);
        auto result = create_expr(e::closure,
                                  create_expr(e::nop),
                                  fn_body,
                                  info);
        info->expr = fn_body;
        if (info->num_bindings) {
          closure_func_info->params.has_static_chain = true;
        }
        closures.exit_closure();
        return result;
      }

      air::exprs::expr * parse_quasi(imms_t body, bool is_result_used) {
        u32_t known_size = 0U;
        u32_t num_exprs = 0U;
        if (!body) {
          return create_expr(e::make_vec, create_expr(e::ld_const, lisp_i64(0)));
        }
        air::exprs::quasi_operation* opers = reinterpret_cast<air::exprs::quasi_operation *>(
          builder.pool->allocate(sizeof(air::exprs::quasi_operation) * body.size()));
        air::exprs::expr * exprs[body.size()];
        bool had_dynamic = false;
        for (auto & item : body) {
          if (item.is_vec_ref() && item.as_vec_ref()->size() != 0) {
            auto vec = item.as_vec_ref()->value();
            if (!vec.front().is_sym_id()
                || vec.front().as_sym_id().value() != env::global_static_ids::id_unquote)
            {
              exprs[num_exprs] = parse_quasi(vec, is_result_used);
              if (exprs[num_exprs]->type != air::expr_type::ld_const) {
                had_dynamic = true;
              }
              opers[num_exprs++] = { air::exprs::quasi_oper_type::push, 1U };
              ++known_size;
              continue;
            }
            auto unquoted = vec[1];
            if (unquoted.is_vec_ref() && unquoted.as_vec_ref()->size()
                && unquoted.as_vec_ref()->value().front().is_sym_id()
                && unquoted.as_vec_ref()->value().front().as_sym_id().value() == env::global_static_ids::id_pre_ellipsis)
            {
              auto unquoted_val = unquoted.as_vec_ref()->value()[1];
              u32_t sub_size = 0U;
              if (lisp_imm * quoted = value_if_quoted(unquoted_val)) {
                if (!is_result_used) {
                  continue;
                }
                J_ASSERT(quoted->is_vec_ref());
                auto vr = quoted->as_vec_ref()->value();
                sub_size = vr.size();
                known_size += sub_size;
              } else {
               had_dynamic = true;
              }
              exprs[num_exprs] = parse_form(unquoted_val, is_result_used);
              opers[num_exprs++] = { air::exprs::quasi_oper_type::append, sub_size };
              continue;
            }
            if (lisp_imm * quoted = value_if_quoted(unquoted)) {
              exprs[num_exprs] = create_expr(e::ld_const, *quoted);
            } else {
              had_dynamic = true;
              exprs[num_exprs] = parse_form(unquoted, is_result_used);
            }
            opers[num_exprs++] = { air::exprs::quasi_oper_type::push, 1U };
            ++known_size;
            continue;
          }
          if (is_result_used) {
            exprs[num_exprs] = create_expr(e::ld_const, item);
            opers[num_exprs++] = { air::exprs::quasi_oper_type::push, 1U };
            ++known_size;
          }
        }
        if (!is_result_used) {
          if (num_exprs == 0) {
            return nullptr;
          } else if (num_exprs == 1) {
            return exprs[0];
          }
          return create_expr(e::progn, span(exprs, num_exprs));
        }

        if (!had_dynamic) {
          auto vec = ctx.allocate_vec(known_size);
          auto cur = vec->begin();
          for (u32_t i = 0U; i < num_exprs; ++i) {
            builder.fn.counts.remove_expr(exprs[i]->type);
            switch (opers[i].type) {
            case air::exprs::quasi_oper_type::push:
              *cur++ = exprs[i]->get_const();
              break;
            case air::exprs::quasi_oper_type::append: {
              auto appended = exprs[i]->get_const().as_vec_ref()->value();
              for (auto & sub : appended) {
                *cur++ = sub;
              }
              exprs[i]->get_const().as_vec_ref()->ext_unref();
              break;
            }
            case air::exprs::quasi_oper_type::none:
              J_FAIL("Error in quasi");
            }
          }
          J_ASSERT(cur == vec->end());
          return create_expr(e::ld_const, vec);
        }

        return create_expr(e::vec_build, span(exprs, num_exprs), span(opers, num_exprs));
      }

      air::exprs::expr * parse_set(imms_t p) {
        check_arg_count(p, 2);
        air::exprs::expr * from = parse_form(p[1], true);
        if (p[0].is_sym_id()) {
          auto to = p[0].as_sym_id();
          J_ASSERT(to);
          if (auto lex = scope.try_find_var_with_scope(to)) {
            if (!lex.is_bound ){
              return create_expr(e::lex_wr, lex.var, from);
            }
            closures.handle_bound_wr(lex);
            return create_expr(e::lex_wr_bound, lex.var, from);
          } else {
            return create_expr(e::sym_val_wr, to, from);
          }
        }
        J_ASSERT(p[0].is_vec_ref());
        auto to_body = p[0].as_vec_ref()->value();
        auto accessor = to_body.pop_front().as_sym_id();
        if (accessor == env::global_static_ids::id_at) {
          check_arg_count(to_body, 2);
          return create_expr(e::range_set,
                             parse_form(to_body[0], true),
                             parse_form(to_body[1], true),
                             from);
        }
        J_FAIL("Unsupported accessor");
      }

      air::exprs::expr * parse_call_like(lisp_vec & vec, bool is_result_used) {
        auto body = vec.value();
        J_ASSERT(body.size() > 0, "Empty vec");
        id symbol = body.pop_front().as_sym_id().value();

        auto old_loc = ctx.source_location();
        auto old_form = current_form;
        current_form = symbol;
        {
          auto new_loc = vec.get_source_location();
          cur_loc.loc = new_loc;
          ctx.source_location() = new_loc;
        }

        auto res = do_parse_call_like(symbol, body, is_result_used);
        current_form = old_form;
        ctx.source_location() = old_loc;
        return res;
      }

      air::exprs::expr * do_parse_call_like(id symbol, imms_t body, bool is_result_used) {
        using gid = env::global_static_ids::id_index;

        symbol = ctx.resolve(symbol, symbol_status::initializing_func);

        if (symbol.package_id() == package_id_global) {
          switch (symbol.index()) {
          case (u32_t)gid::idx_and:
          case (u32_t)gid::idx_or: {
            air::exprs::expr * exprs[body.size()];
            u32_t num_exprs = parse_progn_body(exprs, body, is_result_used, true);
            if (!num_exprs) {
              return is_result_used ? create_expr(e::ld_const, symbol == env::global_static_ids::id_and ? lisp_true_v : lisp_false_v) : nullptr;
            } else if (num_exprs == 1) {
              return exprs[0];
            }
            if (symbol == env::global_static_ids::id_and) {
              return create_expr(e::land, span(exprs, num_exprs));
            } else {
              return create_expr(e::lor, span(exprs, num_exprs));
            }
          }
          case (u32_t)gid::idx_bool:
            return parse_unary_expr(e::to_bool, body, is_result_used);
          case (u32_t)gid::idx_not:
            return parse_unary_expr(e::lnot, body, is_result_used);
          case (u32_t)gid::idx_eq:
            return parse_binary_expr(e::eq, body, is_result_used);
          case (u32_t)gid::idx_neq:
            return parse_binary_expr(e::neq, body, is_result_used);
          case (u32_t)gid::idx_minus:
            if (body.size() == 1U) {
              if (!is_result_used) {
                return parse_form(body.front(), false);
              }
              return create_expr(e::ineg, parse_form(body.front(), true));
            } else {
              return parse_repeated_binary_expr(e::isub, body, is_result_used);
            }
          case (u32_t)gid::idx_plus:
            return parse_repeated_binary_expr(e::iadd, body, is_result_used);
          case (u32_t)gid::idx_multiply:
            return parse_repeated_binary_expr(e::imul, body, is_result_used);
          case (u32_t)gid::idx_divide:
            return parse_repeated_binary_expr(e::idiv, body, is_result_used);
          case (u32_t)gid::idx_call: {
            auto fn = parse_form(body.pop_front(), true);
            u32_t const sz = body.size();
            air::exprs::expr * exprs[sz];
            for (u32_t i = 0U; i < sz; ++i) {
              exprs[i] = parse_form(body[i], true);
            }
            return create_expr(e::call, fn, span(exprs, sz));
          }
          case (u32_t)gid::idx_check_type: {
            auto val = parse_form(body.pop_front(), true);
            return create_expr(e::type_assert, val, parse_type_mask(body));
          }
          case (u32_t)gid::idx_quote: {
            check_arg_count(body, 1);
            return is_result_used ? create_expr(e::ld_const, body[0]) : nullptr;
          }
          case (u32_t)gid::idx_quasiquote: {
            check_arg_count(body, 1);
            return parse_quasi(body[0].as_vec_ref()->value(), is_result_used);
          }
          case (u32_t)gid::idx_vec: {
            return parse_n_ary_expr(e::vec, body, is_result_used);
          }
          case (u32_t)gid::idx_fn: {
            return parse_closure(body, is_result_used);
          }
          case (u32_t)gid::idx_at:
            return parse_binary_expr(e::range_get, body, is_result_used);
          case (u32_t)gid::idx_length:
            return parse_unary_expr(e::range_length, body, is_result_used);
          case (u32_t)gid::idx_copy:
            return parse_unary_expr(e::range_copy, body, is_result_used);
          case (u32_t)gid::idx_if: {
            auto oper = body.pop_front();
            auto true_branch = parse_form(body.pop_front(), is_result_used);
            auto false_branch = parse_progn(body, is_result_used);
            if (!true_branch && !false_branch) {
              return parse_form(oper, false);
            }
            if (!true_branch) {
              true_branch = create_expr(e::nop);
            } else if (!false_branch) {
              false_branch = create_expr(e::nop);
            }
            return create_expr(e::b_if,
                               parse_form(oper, true),
                               true_branch,
                               false_branch);
          }
          case (u32_t)gid::idx_do_until: {
            scope.enter_loop();
            auto result = create_expr(e::do_until, parse_progn(body, true));
            scope.exit_loop();
            return result;
          }
          case (u32_t)gid::idx_make_vec:
            return parse_unary_expr(e::make_vec, body, is_result_used);
          case (u32_t)gid::idx_let:
            return parse_let(body, is_result_used);
          case (u32_t)gid::idx_set:
            return parse_set(body);
          case (u32_t)gid::idx_setp: {
            u32_t sz = body.size();
            air::exprs::expr * exprs[sz];
            u32_t i = 0;
            for (auto & form : body) {
              exprs[i++] = parse_set(form.as_vec_ref()->value());
            }
            return create_expr(e::progn, span(exprs, sz));
          }
          case (u32_t)gid::idx_progn:
            return parse_progn(body, is_result_used);
          case (u32_t)gid::idx_append:
            return parse_n_ary_expr(e::vec_append, body, is_result_used);
          default:
            break;
          }
        }

        auto sym = ctx.get_symbol(symbol, compilation::initializing_func);

        auto res = compilation::macroexpand_step(sym, body);
        if (res.status == macroexpand_status::pending) {
          /// \todo Better error
          throw_symbol_error(function_not_found, symbol);
        } else if (res.status == macroexpand_status::did_expand) {
          return parse_form(res.result, is_result_used);
        }

        if (!sym->is_constant() || !sym->value_info.is_immutable()) {
          J_TODO();
        }

        return parse_n_ary_expr(e::full_call, body, true, symbol);
      }

      air::exprs::expr * parse_form(lisp_imm & form, bool is_result_used) {
        switch (form.tag()) {
        //Self-evaluating forms (i.e. constants):
        case tag_i64:
        case tag_f32:
        case tag_bool:
        case tag_nil:
        case tag_str_ref:
          if (!is_result_used) {
            return nullptr;
          }
          return create_expr(e::ld_const, form);

        case tag_sym_id: {
          id symbol_name = form.as_sym_id();
          J_ASSERT(symbol_name);
          if (auto lex = scope.try_find_var_with_scope(symbol_name)) {
            if (!is_result_used) {
              return nullptr;
            }
            if (!lex.is_bound) {
              return create_expr(e::lex_rd, lex.var);
            }
            closures.handle_bound_rd(lex);
            return create_expr(e::lex_rd_bound, lex.var);
          } else {
            return create_expr(e::sym_val_rd, symbol_name);
          }
        }
        case tag_vec_ref: {
          return parse_call_like(*form.as_vec_ref().value(), is_result_used);
        }
        default:
          J_THROW("Invalid form - type '{}' is not alowed.", form.tag());
        }
      }

      air::exprs::expr * parse_let(imms_t body, bool is_result_used) {
        J_ASSERT(body.front().is_vec_ref());
        auto bindings_form = body.pop_front();
        const u32_t num_bindings = bindings_form.as_vec_ref()->size();
        J_ASSERT(num_bindings);
        auto bindings = alloc_vars(num_bindings);
        u32_t i = 0U;
        for (auto & binding : bindings_form.as_vec_ref()->value()) {
          bindings[i++] = air::air_var{
            binding.is_sym_id() ? binding.as_sym_id().value() : binding.as_vec_ref()->value().front().as_sym_id().value(),
            builder.fn.num_lex_vars++,
          };
        }

        i = 0U;
        auto let = create_expr(e::let, air::exprs::allocate_inputs(body.size() + num_bindings), bindings);
        auto s = scope.enter_scope(let);
        air::air_var * air_var = bindings.begin();
        for (auto & binding : bindings_form.as_vec_ref()->value()) {
          air::exprs::expr * init_expr = nullptr;
          if (binding.is_sym_id()) {
            init_expr = create_expr(e::nop);
          } else {
            init_expr = parse_form(binding.as_vec_ref()->value()[1], true);
          }
          builder.create_late(let, i, e::lex_wr, with_metadata(mdi::loc{ctx.source_location()}), air_var, init_expr);
          ++i, ++air_var;
        }

        air::exprs::expr * body_exprs[body.size()];
        u32_t num_body = parse_progn_body(body_exprs, body, is_result_used);
        if (!num_body) {
          builder.create_late(let, i++, e::nop, with_metadata(mdi::loc{ctx.source_location()}));
        } else {
          for (u32_t j = 0; j < num_body; ++j) {
            J_ASSERT_NOT_NULL(body_exprs[j]);
            builder.attach_to(let, i++, body_exprs[j]);
          }
        }
        let->num_inputs = i;
        return let;
      }


      u32_t parse_progn_body(air::exprs::expr ** J_NOT_NULL target_array,
                             imms_t body,
                             bool is_result_used,
                             bool is_head_used = false)
      {
        if (body.empty()) {
          return 0U;
        }
        u32_t num_exprs = 0U;

        auto last = body.pop_back();
        for (auto & form : body) {
          if (air::exprs::expr * expr = parse_form(form, is_head_used)) {
            target_array[num_exprs++] = expr;
          }
        }
        if (air::exprs::expr * expr = parse_form(last, is_result_used)) {
          target_array[num_exprs++] = expr;
        }
        return num_exprs;
      }

      air::exprs::expr * parse_progn(imms_t body, bool is_result_used) {
        air::exprs::expr * exprs[body.size()];
        u32_t num_exprs = parse_progn_body(exprs, body, is_result_used);
        if (!num_exprs) {
          return is_result_used ? create_expr(e::nop) : nullptr;
        } else if (num_exprs == 1) {
          return exprs[0];
        }
        return create_expr(e::progn, span<air::exprs::expr * const>(exprs, num_exprs));
      }

      J_RETURNS_NONNULL air::exprs::expr * parse_fn_body(imms_t body, lisp::functions::func_info * J_NOT_NULL func_info, bool is_closure) {
        const u32_t num_params = func_info->params.num_params;
        auto air_vars = alloc_vars(num_params);
        for (u32_t i = 0U; i < num_params; ++i) {
          air_vars[i] = air::air_var(func_info->params[i], builder.fn.num_lex_vars++);
        }

        auto fn_body = create_expr(e::fn_body, air::exprs::allocate_inputs(num_params + body.size() + is_closure), air_vars);
        auto s = scope.enter_scope(fn_body);
        if (is_closure) {
          builder.create_late(fn_body, 0, e::nop, with_metadata(mdi::loc{ctx.source_location()}));
        }
        u32_t i = is_closure;
        u32_t arg_i = 0U;
        air::air_var * air_var = air_vars.begin();
        for (const u32_t rest_index = num_params - func_info->params.has_rest;
             arg_i < num_params; ++i, ++air_var, ++arg_i) {
          builder.create_late(
            fn_body, i,
            e::lex_wr,
            with_metadata(mdi::loc{ctx.source_location()}),
            air_var,
            arg_i == rest_index ? create_expr(e::fn_rest_arg) : create_expr(e::fn_arg, arg_i));
        }


        auto last = body.pop_back();
        for (auto & form : body) {
          if (air::exprs::expr * expr = parse_form(form, false)) {
            builder.attach_to(fn_body, i++, expr);
          }
        }
        builder.attach_to(fn_body, i++, parse_form(last, true));
        fn_body->num_inputs = i;
        return fn_body;
      }
    };
  }

  [[nodiscard]] J_RETURNS_NONNULL air::air_function * top_level_defun_to_air(
    compilation::context & ctx,
    imms_t body)
  {
    auto afn = &ctx.temp_pool.emplace<air::air_function>();
    ast_visitor visitor(ctx, afn);
    afn->body = visitor.parse_fn_body(body, ctx.func_info, false);
    air::run_air_passes(ctx, &ctx.temp_pool, afn);
    return afn;
  }
}
