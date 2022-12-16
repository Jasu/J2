#include "link_function.hpp"
#include "lisp/cir_to_mc/mc_function.hpp"
#include "lisp/symbols/symbol.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/env/environment.hpp"
#include "context.hpp"

namespace j::lisp::compilation {
  namespace {
    void apply_reloc(context & ctx,
                     const assembly::reloc & reloc,
                     void * J_NOT_NULL begin,
                     const void * J_NOT_NULL final_location,
                     const void * constants_begin) {
      const void * reloc_target = nullptr;
      switch (reloc.source_type) {
      case assembly::reloc_source_type::none:
        J_FAIL("Reloc type is none.");
      case assembly::reloc_source_type::fn_addr: {
        J_ASSUME(reloc.cc != functions::calling_convention::none);
        auto fn = ctx.get_symbol(reloc.source_id, compilation::initializing_func);
        reloc_target = fn->value_info.func_info->cc_info[reloc.cc].fn_ptr;
        J_ASSERT(reloc_target);
        break;
      }
      case assembly::reloc_source_type::var_addr:
        reloc_target = &ctx.get_symbol(reloc.source_id, compilation::initializing_var)->value;
        break;
      case assembly::reloc_source_type::var_value_tagged:
        reloc_target = (void*)ctx.get_symbol(reloc.source_id, compilation::initialized_var)->value.raw;
        break;
      case assembly::reloc_source_type::var_value_untagged: {
        auto val = ctx.get_symbol(reloc.source_id, compilation::initialized_var)->value;
        switch (val.type()) {
        case imm_undefined: J_FAIL("Undefined lisp_imm in relocation");
        case imm_f32:
        case imm_bool:
        case imm_nil:
        case imm_sym_id:
        case imm_act_rec:
        case imm_range:
          J_FAIL("Unsupported type");
        case imm_vec_ref:
          reloc_target = (void*)(val.raw - (u64_t)tag_vec_ref);
          break;
        case imm_fn_ref:
          reloc_target = (void*)(val.raw - (u64_t)tag_fn_ref);
          break;
        case imm_closure_ref:
          reloc_target = (void*)(val.raw - (u64_t)tag_closure_ref);
          break;
        case imm_str_ref:
          reloc_target = (void*)(val.raw - (u64_t)tag_str_ref);
          break;
        case imm_rec_ref:
          reloc_target = (void*)(val.raw - (u64_t)tag_rec_ref);
          break;
        case imm_i64:
          reloc_target = (void*)(u64_t)(val.raw >> 1);
          break;
        }
        break;
      }
      case assembly::reloc_source_type::constant_addr:
        J_ASSUME_NOT_NULL(constants_begin);
        reloc_target = add_bytes(constants_begin, reloc.constant_offset);
        break;
      }

      J_ASSUME_NOT_NULL(reloc_target);
      assembly::apply_reloc(begin, final_location, reloc.opts, reloc_target);
    }

    [[nodiscard]] J_RETURNS_NONNULL void * link_single(context & ctx, lisp::mem::code_heap::allocation & alloc, const assembly::function & asm_function) {
      // Copy the unrelocated code to a temporary buffer.
      j::mem::buffer code(j::mem::allocate_tag, asm_function.code.size());
      asm_function.code.data.copy_to(code);
      j::mem::buffer constants;
      const void * code_begin = alloc.code_begin();
      const void * data_begin = nullptr;
      if (asm_function.constants.size()) {
        constants = j::mem::buffer(j::mem::allocate_tag, asm_function.constants.size());
        data_begin = alloc.data_begin();
        asm_function.constants.data.copy_to(constants);
      }

      for (auto & reloc : asm_function.code.relocs) {
        apply_reloc(ctx, reloc, add_bytes(code.begin(), reloc.offset), add_bytes(code_begin, reloc.offset), data_begin);
      }
      if (asm_function.constants.size()) {
        J_ASSUME_NOT_NULL(data_begin);
        for (auto & reloc : asm_function.constants.relocs) {
          apply_reloc(ctx, reloc, add_bytes(constants.begin(), reloc.offset), add_bytes(data_begin, reloc.offset), data_begin);
        }
      }

      alloc.assign(code, constants);
      return (void*)code_begin;
    }
  }

  void allocate_function(context & ctx, functions::func_info * info, const cir_to_mc::mc_functions * mc_functions) {
    J_ASSERT(mc_functions);
    info->cc_info[functions::calling_convention::abi_call].allocation =
      ctx.env_compiler.env->code_heap.allocate(mc_functions->asm_funcs[functions::calling_convention::abi_call].code.size(),
                                               mc_functions->asm_funcs[functions::calling_convention::abi_call].constants.size());

    info->cc_info[functions::calling_convention::full_call].allocation =
      ctx.env_compiler.env->code_heap.allocate(mc_functions->asm_funcs[functions::calling_convention::full_call].code.size(),
                                               mc_functions->asm_funcs[functions::calling_convention::full_call].constants.size());

    if (!mc_functions->num_closures) {
      return;
    }
    auto closures = (functions::closure_info*)ctx.metadata_pool().allocate(sizeof(functions::closure_info) * mc_functions->num_closures);
    info->closures = {closures, mc_functions->num_closures};

    auto cur_closure = closures;
    for (auto & closure : mc_functions->closures()) {
      J_ASSERT_NOT_NULL(closure.symbol);
      auto & asmfn = closure.asm_funcs[functions::calling_convention::full_call];
      auto c = ::new (cur_closure) functions::closure_info{closure.symbol, {}};
      auto & cc_info = c->cc_info[functions::calling_convention::full_call];
      cc_info.allocation = ctx.env_compiler.env->code_heap.allocate(asmfn.code.size(), asmfn.constants.size());
      ::j::memcpy(&closure.symbol->value_info.func_info->cc_info[functions::calling_convention::full_call].allocation, &cc_info.allocation, sizeof(cc_info.allocation));
      cc_info.asm_info = asmfn.info;
      cur_closure++;
    }
  }

  void link_function(context & ctx, functions::func_info * info, const cir_to_mc::mc_functions * mc_functions) {
    J_ASSERT(mc_functions);
    auto cur_closure = info->closures.begin();
    for (auto & closure : mc_functions->closures()) {
      J_ASSERT_NOT_NULL(closure.symbol);
      auto & cc_info = cur_closure->cc_info[functions::calling_convention::full_call];
      cc_info.fn_ptr = link_single(ctx, cc_info.allocation, closure.asm_funcs[functions::calling_convention::full_call]);
      cc_info.asm_info->register_unwind(cc_info.fn_ptr);
      cur_closure->sym->value = lisp_fn_ref((lisp_fn_ptr_t)cc_info.fn_ptr);
      cur_closure->sym->value_info.func_info->cc_info[functions::calling_convention::full_call].fn_ptr = cc_info.fn_ptr;
      cur_closure->sym->value_info.func_info->cc_info[functions::calling_convention::full_call].asm_info = cc_info.asm_info;
      cur_closure++;
    }
    for (auto cc : functions::ccs) {
      auto & cc_info = info->cc_info[cc];
      cc_info.fn_ptr = link_single(ctx, cc_info.allocation, mc_functions->asm_funcs[cc]);
      cc_info.asm_info->register_unwind(cc_info.fn_ptr);
    }
    ctx.symbol.value = lisp_fn_ref((lisp_fn_ptr_t)info->cc_info[functions::calling_convention::full_call].fn_ptr);
  }
}
