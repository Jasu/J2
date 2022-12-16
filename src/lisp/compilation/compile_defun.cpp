#include "lisp/compilation/compile_defun.hpp"

#include "lisp/compilation/context.hpp"
#include "lisp/compilation/link_function.hpp"
#include "lisp/ast_to_air/ast_to_air.hpp"
#include "lisp/env/static_ids.hpp"
#include "lisp/air_to_cir/air_to_cir.hpp"
#include "lisp/cir_to_mc/cir_to_mc.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/symbols/symbol.hpp"
#include "mem/bump_pool.hpp"
#include "debug/timing.hpp"

J_DEFINE_TIMERS("Compile defun", defun_total, ast_to_air, air_to_cir, cir_to_mc, defun_alloc, defun_link)

namespace j::lisp::compilation {
  void compile_defun(context & ctx) {
    auto full = ctx.symbol.value.as_vec_ref()->value();
    auto old_loc = ctx.source_location();
    auto new_loc = ctx.symbol.value.as_vec_ref()->get_source_location();
    if (new_loc) {
      ctx.source_location() = new_loc;
    }
    compile_defun(ctx,
                  full.at(2).as_vec_ref()->value(),
                  full.without_prefix(3));
    ctx.source_location() = old_loc;
  }

  functions::func_info * parse_func_info(context & ctx, imms_t param_list) {
    const u32_t num_args = param_list.size();
    functions::func_info * fn_info = &ctx.metadata_pool().emplace_with_padding<functions::func_info>(sizeof(id) * num_args);
    id * arg_out = reinterpret_cast<id*>(fn_info + 1);
    J_ASSERT(arg_out == fn_info->params.begin());
    fn_info->params.num_params = num_args;
    for (lisp_imm & arg_in : param_list) {
      if (arg_in.is_sym_id()) {
        *arg_out++ = arg_in.as_sym_id();
      } else {
        J_ASSUME(fn_info->params.has_rest == false);
        fn_info->params.has_rest = true;
        auto arg_vec = arg_in.as_vec_ref();
        J_ASSERT(arg_vec->at(0).as_sym_id() == env::global_static_ids::id_post_ellipsis);
        *arg_out++ = arg_vec->at(1).as_sym_id();
      }
    }
    return fn_info;
  }

  namespace {
    void compile_defun_tail(context & ctx, functions::func_info * J_NOT_NULL fn_info, air::air_function * J_NOT_NULL air_fn) {
    J_START_TIMER(air_to_cir);
    auto cir_fn = air_to_cir::air_to_cir(ctx, air_fn);
    J_STOP_TIMER(air_to_cir);
    J_START_TIMER(cir_to_mc);
    auto mc_fn = cir_to_mc::cir_to_mc(ctx, cir_fn);
    J_STOP_TIMER(cir_to_mc);
    J_START_TIMER(defun_alloc);
    allocate_function(ctx, fn_info, mc_fn);
    J_STOP_TIMER(defun_alloc);
    J_START_TIMER(defun_link);
    link_function(ctx, fn_info, mc_fn);
    J_STOP_TIMER(defun_link);
    for (auto & c : fn_info->closures) {
      c.sym->set_initialized();
    }
    J_STOP_TIMER(defun_total);
    }
  }

  void compile_defun(context & ctx, imms_t param_list, imms_t vec_body) {
    J_START_TIMER(defun_total);
    functions::func_info * fn_info = parse_func_info(ctx, param_list);
    ctx.symbol.value_info.func_info = fn_info;
    ctx.func_info = fn_info;
    J_START_TIMER(ast_to_air);
    auto air_fn = ast_to_air::top_level_defun_to_air(ctx, vec_body);
    J_STOP_TIMER(ast_to_air);
    compile_defun_tail(ctx, fn_info, air_fn);
  }

  void compile_defun(context & ctx, const prototype_init & proto, air::air_function * J_NOT_NULL air_fn) {
    J_START_TIMER(defun_total);
    const u32_t num_args = proto.params.size();
    functions::func_info * fn_info = &ctx.metadata_pool().emplace_with_padding<functions::func_info>(sizeof(id) * num_args);
    fn_info->params.num_params = num_args;
    j::memcpy(fn_info + 1, proto.params.begin(), sizeof(id) * num_args);
    fn_info->params.has_rest = proto.has_rest;
    ctx.symbol.value_info.func_info = fn_info;
    ctx.func_info = fn_info;

    compile_defun_tail(ctx, fn_info, air_fn);
  }
}
