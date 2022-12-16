#include "lisp/air/passes/const_fold.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/air/exprs/expr_init_data_map.hpp"
#include "lisp/air/exprs/expr_builder.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/air/exprs/expr_data.hpp"
#include "lisp/compilation/compile_defun.hpp"
#include "lisp/values/static_objects.hpp"
#include "lisp/assembly/target.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "lisp/env/compilation_options.hpp"

namespace j::lisp::air::inline passes {
  namespace {
    constinit const static_lisp_str name_val{"val"};
    constinit const static_lisp_str name_lhs{"lhs"};
    constinit const static_lisp_str name_rhs{"rhs"};

    constexpr inline const char * const disabled_passes[]{
      "validation-initial",
      "trs-simplify",
      "count-exprs-trs-simplify",
      "validation-trs-simplify",
      "dce-1",
      "validation-dce-1",
      "reaching-defs",
      "validation-reaching-defs",
      "dce-2",
      "validation-dce-2",
      "lex-vars",
      "validation-lex-vars",
      "validation-propagate-types-1",
      "act-rec-compute",
      "validation-act-rec-compute",
      "act-rec-convert",
      "count-exprs-act-rec-convert",
      "validation-act-rec-convert",
      "simplify",
      "count-exprs-simplify",
      "validation-simplify",
      "propagate-barriers",
      "validation-propagate-barriers",
      "reassociate",
      "validation-reassociate",
      "validation-propagate-reps",
      "validation-convert-reps",
      "validation-propagate-types-2",
    };
  }

  J_A(RNN,NODISC) packages::pkg * create_const_fold_pkg(env::environment * J_NOT_NULL e) {
    packages::pkg * pkg = e->package_at(package_id_const_fold);
    auto tgt = assembly::target::get_target(assembly::target_name::amd64_ia64);
    auto asmctx = assembly::asm_context.enter(tgt);

    env::compilation_options opts = env::compilation_options_nodebug;
    opts.target = tgt;
    for (const char * pass : disabled_passes) {
      opts.air_options.configure_pass(pass, { .enabled = false });
    }
    auto envc = e->get_env_compiler(&opts);

    id id_lhs = pkg->symbol_table.names.emplace(name_lhs);
    id id_rhs = pkg->symbol_table.names.emplace(name_rhs);
    id id_val = pkg->symbol_table.names.emplace(name_val);
    id lhs_rhs_params[]{id_lhs, id_rhs};
    id id_params[]{id_val};

    compilation::prototype_init init_unary{.params{id_params}};
    compilation::prototype_init init_binary{.params{lhs_rhs_params}};

    auto & pkg_context = pkg->get_pkg_context();

    exprs::expr * fn_body = J_ALLOCA_NEW_PADDING(exprs::expr, sizeof(exprs::input) + sizeof(air_var_defs_t)){
      .type = expr_type::fn_body,
      .num_inputs = 1,
      .max_inputs = 1,
      .result = val_imm_tag,
    };
    ::new (fn_body->static_begin()) air_var_defs_t{};

    exprs::expr * expr = J_ALLOCA_NEW_PADDING(exprs::expr, sizeof(exprs::input) * 2){
      .max_inputs = 2,
    };

    exprs::expr * expr_arg_0 = J_ALLOCA_NEW_PADDING(exprs::expr, sizeof(u32_t)){
      .type = expr_type::fn_arg,
      .result = val_imm_tag,
    };
    *(u32_t*)expr_arg_0->static_begin() = 0U;

    exprs::expr * expr_arg_1 = J_ALLOCA_NEW_PADDING(exprs::expr, sizeof(u32_t)){
      .type = expr_type::fn_arg,
      .result = val_imm_tag,
    };
    *(u32_t*)expr_arg_1->static_begin() = 1U;


    air_function afn{fn_body};
    exprs::expr_builder builder{nullptr, afn};
    u8_t index = 0;
    for (auto & ed : exprs::expr_data_v) {
      const expr_type t = (expr_type)(index++);
      if (!ed.fold.num_args) {
        continue;
      }
      expr->num_inputs = ed.fold.num_args;
      builder.attach_to(fn_body, 0, expr);
      fn_body->result = val_imm_tag;
      fn_body->input(0).type = val_imm_tag;

      builder.attach_to(expr, 0, expr_arg_0);
      expr_arg_0->result = val_imm_tag;

      auto & d = exprs::expr_init_data_v[(u8_t)t];
      expr->type = t;
      expr->result = d.result;
      expr->input(0).type = d.arg(0);
      fn_body->visit_flag = false;


      auto & proto = ed.fold.num_args == 1 ? init_unary : init_binary;
      id fn_name = pkg->symbol_table.names.emplace(expr_type_lisp_names[(u8_t)t]);
      proto.fn_name = fn_name;

      auto sym = pkg->symbol_table.declare(fn_name, lisp_nil{}, symbols::symbol_flag::constant,
                                           value_info::value_info(imm_fn_ref, value_info::function_value_flags));
      compilation::context ctx(pkg_context, *sym);
      if (ed.fold.num_args > 1) {
        J_ASSERT(ed.fold.num_args == 2);
        builder.attach_to(expr, 1, expr_arg_1);
        expr_arg_1->result = val_imm_tag;
        expr->input(1).type = d.arg(1);
      }

      run_air_passes(ctx, &ctx.temp_pool, &afn);
      compilation::compile_defun(ctx, proto, &afn);
    }

    ::delete envc;
    e->m_env_compiler = nullptr;
    return pkg;
  }

  namespace {
    J_A(NODISC,RNN) inline void * get_fn(packages::pkg * J_NOT_NULL pkg, expr_type t) noexcept {
      auto sym = pkg->symbol_table.at(pkg->symbol_table.names.id_of(expr_type_names[(u8_t)t]));
      J_ASSERT(sym && sym->value.is_fn_ref());
      auto fn_ptr = sym->value_info.func_info->cc_info[lisp::functions::calling_convention::abi_call].fn_ptr;
      J_ASSERT(fn_ptr);
      return fn_ptr;
    }
  }

  J_A(RNN,NODISC) const const_fold_table * get_const_fold_table(env::environment * J_NOT_NULL e) {
    if (!e->air_const_fold_table) {
      auto pkg = e->packages[package_id_const_fold];
      J_ASSERT_NOT_NULL(pkg);

      auto tbl = ::new const_fold_table;
      e->air_const_fold_table = tbl;
      auto * wr = tbl->folders;
      for (i32_t i = 0U; i < num_expr_types_v; ++i, ++wr) {
        expr_type t = (expr_type)i;
        auto & ed = exprs::expr_data_v[t].fold;
        if (ed.num_args) {
          auto fn = get_fn(pkg, t);
          if (ed.num_args == 1) {
            wr->fn_1 = reinterpret_cast<lisp_abi_call_1_ptr_t>(fn);
          } else {
            wr->fn_2 = reinterpret_cast<lisp_abi_call_2_ptr_t>(fn);
          }

          if (ed.tail_fold_type != expr_type::none) {
            J_ASSERT(exprs::expr_data_v[ed.tail_fold_type].fold.num_args == 2);
            wr->fn_tail = reinterpret_cast<lisp_abi_call_2_ptr_t>(get_fn(pkg, ed.tail_fold_type));
          }
        }
      }
    }
    return e->air_const_fold_table;
  }
}
