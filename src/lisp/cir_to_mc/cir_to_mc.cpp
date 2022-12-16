#include "lisp/cir_to_mc/cir_to_mc.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/cir_to_mc/operands.hpp"
#include "lisp/cir_to_mc/utils.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/functions/calling_convention.hpp"
#include "lisp/assembly/register_info.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/cir/cir_function.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/assembly/amd64/abi/ia64_target.hpp"
#include "lisp/assembly/compiler.hpp"
#include "lisp/common/metadata_init.hpp"
#include "strings/format.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace j::lisp::cir_to_mc {
  namespace {

    namespace c = cir;
    namespace o = cir::ops::defs;
    namespace a = assembly;

    constexpr inline a::integer_binop to_integer_binop(c::op_type op_type) noexcept {
      J_ASSERT(op_type >= c::op_type::iadd && op_type <= c::op_type::sar);
      return (a::integer_binop)((u8_t)op_type - (u8_t)c::op_type::iadd);
    }

    constexpr inline a::integer_unop to_integer_unop(c::op_type op_type) noexcept {
      J_ASSERT(op_type >= c::op_type::ineg && op_type <= c::op_type::bnot);
      return (a::integer_unop)((u8_t)op_type - (u8_t)c::op_type::ineg);
    }

    template<c::op_type... Types>
    struct J_TYPE_HIDDEN one_of final {
      template<c::op_type Type> requires ((Type == Types) || ...)
      J_ALWAYS_INLINE_NO_DEBUG constexpr one_of(const value_tag<Type> &) noexcept {}
    };

    struct J_TYPE_HIDDEN visitor {
      a::compiler * c;
      cir::cir_ssa * ssa;
      cir::bb * bb;
      cir::bb * next_bb;
      const a::reg_info * reg_info;
      u32_t label_counter = 0;

      using op_type J_NO_DEBUG_TYPE = cir::op_type;

      J_ALWAYS_INLINE void operator()(one_of<op_type::icopy, op_type::fcopy>, const c::op * o, cir::loc result, const cir::input & value) {
        c->emit_mov(to_asm_reg(result), to_asm_operand(c, value), copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::iconst>, const c::op * o, cir::loc to, u64_t value) {
        a::reg to_reg;
        a::operand from;
        if (value <= U32_MAX) {
          to_reg = to_asm_reg32(to);
          from = a::imm32(value);
        } else {
          to_reg = to_asm_reg64(to);
          if ((i64_t)value >= I32_MIN && (i64_t)value < 0) {
            from = a::imm32((i32_t)value);
          } else {
            from = a::rel64(a::reloc_source{
                .source_type = a::reloc_source_type::constant_addr,
                .constant_offset = c->builder->add_constant(value)
              });
          }
        }
        c->emit_mov(to_reg, from, copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::mconst>, const c::op * o, cir::loc to, j::mem::const_memory_region reg) {
        c->emit_lea(
          to_asm_reg(to),
          a::rel64(
            a::reloc_source{
              .source_type = a::reloc_source_type::constant_addr,
              .constant_offset = c->builder->add_constant(reg)
            }, -4),
          copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::fconst>, const c::op *, cir::loc, float) {
        J_TODO();
      }

      void operator()(value_tag<op_type::mov>, const c::op * o, const cir::op_result_input & in) {
        cir::loc from = in.use.loc_in;
        cir::loc to = in.use.loc_out;
        strings::string comment;
        cir::loc spill{};
        if (to.is_reg() && from.is_spill_slot()) {
          comment = "Unspill ";
          spill = from;
        } else if (to.is_spill_slot() && from.is_reg()) {
          comment = "Spill ";
          spill = to;
        }
        comment += strings::format("{}", spill);
        c->emit_mov(to_asm_operand(c, to), to_asm_operand(c, from), with_metadata(mdi::comment(comment), o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::swap>, const c::op * o, cir::loc to, cir::loc from) {
        c->emit_swap(to_asm_reg(to), to_asm_reg(from), copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::push>, const c::op * o, mem_width type, cir::loc reg) {
        c->emit_push(to_asm_operand(c, reg, type), copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::pop>, const c::op * o, mem_width type, cir::loc reg) {
        c->emit_pop(to_asm_operand(c, reg, type), copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::type_error>, const c::op * o, imm_type_mask mask) {
        c->emit_type_check_info(mask);
        c->emit_trap(a::trap_type::type_check, copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::debug_trap>, const c::op * o) {
        c->emit_trap(a::trap_type::debug, copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::fn_enter>, const c::op * o) {
        c->emit_enter_stack_frame(ssa->num_spill_slots, with_metadata(mdi::comment("Begin frame"), o->metadata()));
      }

      J_ALWAYS_INLINE_NO_DEBUG constexpr void operator()(one_of<op_type::fn_iarg, op_type::fn_farg,
                                      op_type::fn_rest_ptr,op_type::fn_rest_len,
                                      op_type::fn_sc_ptr>, const c::op *) const noexcept
      { }

      void operator()(one_of<op_type::iret, op_type::fret>, const c::op * o) {
        c->emit_leave_stack_frame(ssa->num_spill_slots, with_metadata(mdi::comment("Leave frame"), o->metadata()));
        c->emit_return(copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(one_of<op_type::ineg, op_type::bnot>, const c::op * o, cir::loc result, const cir::input & value) {
        c->emit_op(to_integer_unop(o->type), to_asm_reg(result), to_asm_operand(c, value), copy_metadata(o->metadata()));
      }

      void operator()(one_of<
                      op_type::iadd, op_type::isub,
                      op_type::smul, op_type::sdiv, op_type::umul, op_type::udiv,
                      op_type::band, op_type::bor, op_type::bxor,
                      op_type::shl, op_type::shr, op_type::sar>,
                      const c::op * o, cir::loc result, const cir::input & lhs, const cir::input & rhs) {
        c->emit_op(to_integer_binop(o->type),
                   to_asm_reg(result),
                   to_asm_operand(c, lhs),
                   to_asm_operand(c, rhs),
                   copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::alloca>, const c::op * o, cir::loc result, span<const cir::input> inputs) {
        c->emit_alloc_full_call(a::imm8(inputs.size()), copy_metadata(o->metadata()));
        for (u32_t i = 0; i < inputs.size(); ++i) {
          c->emit_write_full_call_arg(i, to_asm_operand(c, inputs[i]), copy_metadata(o->metadata()));
        }
        c->emit_load_sp(to_asm_reg(result), with_metadata(mdi::result("ArgArray"), mdi::copy(o->metadata())));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::dealloca>, const c::op * o, const cir::input & num_args) {
        c->emit_dealloc_full_call(to_asm_operand(c, num_args), copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::full_call>, const c::op * o, cir::loc, const cir::input & fn, const cir::input &, const cir::input &) {
        c->emit_call(to_branch_operand(fn), copy_metadata(o->metadata()));
      }

      void operator()(value_tag<op_type::call>, const c::op * o, cir::loc, const cir::input & fn, span<const cir::input> inputs) {
        if (inputs) {
          c->emit_alloc_full_call(a::imm8(inputs.size()), with_metadata(mdi::result("ArgArray"), mdi::op(0, "NArgs"), mdi::copy(o->metadata())));
          for (u32_t i = 0; i < inputs.size(); ++i) {
            c->emit_write_full_call_arg(i, to_asm_operand(c, inputs[i]), copy_metadata(o->metadata()));
          }
          c->emit_load_sp(a::reg64(reg_info->gpr_arg_regs[0]), copy_metadata(o->metadata()));
        }
        c->emit_mov(a::reg32(reg_info->gpr_arg_regs[1]), a::imm32(inputs.size()), with_metadata(mdi::op(0, "NArgs"), mdi::copy(o->metadata())));
        c->emit_call(to_asm_mem_operand(fn), copy_metadata(o->metadata()));
        if (inputs) {
          c->emit_dealloc_full_call(a::imm8(inputs.size()), with_metadata(mdi::op(0, "NArgs"), mdi::copy(o->metadata())));
        }
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::abi_call>, const c::op * o, cir::loc, const cir::input & fn, span<const cir::input>) {
        c->emit_call(to_branch_operand(fn), copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::mem_ird>, const c::op * o, cir::loc result, const cir::input & ptr) {
        c->emit_mov(to_asm_reg(result), to_asm_mem_operand(ptr), copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::lea>, const c::op * o, cir::loc result, const cir::input & addr) {
        c->emit_lea(to_asm_reg(result), to_asm_mem_operand(addr), copy_metadata(o->metadata()));
      }

      a::operand maybe_make_constant_operand(const cir::input & value) {
        if (!value.is_constant()) {
          return to_asm_operand(c, value);
        }
        auto cn = value.const_data;

        if (cn < U32_MAX && !(cn & (1U << 31))) {
          return a::imm32(cn);
        }

        return a::rel64(
          a::reloc_source{
            .source_type = a::reloc_source_type::constant_addr,
            .constant_offset = c->builder->add_constant(cn)
          }, -4);
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::mem_iwr>, const c::op * o, const cir::input & ptr, const cir::input & value) {
        c->emit_mov(to_asm_mem_operand(ptr), maybe_make_constant_operand(value), copy_metadata(o->metadata()));
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::mem_frd>, const c::op *, cir::loc, const cir::input &) {
        J_TODO();
      }

      J_ALWAYS_INLINE void operator()(value_tag<op_type::mem_fwr>, const c::op *, const cir::input &, const cir::input &) {
        J_TODO();
      }

      void operator()(value_tag<op_type::mem_copy64>, const c::op *,
                      const cir::input & to,
                      const cir::input & from,
                      const cir::input & count) {
        c->emit_copy_qwords(to_asm_reg(to), to_asm_reg(from), to_asm_reg(count, mem_width::dword));
      }

      void operator()(one_of<op_type::eq, op_type::neq>, const c::op * J_NOT_NULL o, cir::loc result, const cir::input & lhs, const cir::input & rhs) {
        const bool invert = o->type == op_type::neq;
        auto result_reg = to_asm_reg(result, mem_width::byte);
        c->emit_cmp_get(invert ? a::cmp_flag_condition::neq : a::cmp_flag_condition::eq,
                        result_reg, to_asm_operand(c, lhs), to_asm_operand(c, rhs),
                        with_metadata(mdi::copy(o->metadata()), mdi::comment(invert ? "Not equal" : "Equal")));
        c->emit_shl(result_reg, result_reg, a::imm8(6), with_metadata(mdi::op(1, "OffsetTrue"), mdi::copy(o->metadata())));
        c->emit_iadd(result_reg, result_reg, a::imm8((u8_t)tag_bool), with_metadata(mdi::op(1, "False"), mdi::copy(o->metadata())));
      }

      J_ALWAYS_INLINE void operator()(one_of<op_type::fadd, op_type::fsub>, const c::op *, cir::loc, const cir::input &, const cir::input &) const {
        J_TODO();
      }

      void operator()(value_tag<op_type::jmp>, const c::op * o, const cir::bb * J_NOT_NULL exit) {
        if (exit != next_bb) {
          c->emit_jmp(exit->asm_label, copy_metadata(o->metadata()));
        }
      }

      void operator()(value_tag<op_type::tag>, const c::op * o, cir::loc result, imm_type type, const cir::input & v) {
        auto to = to_asm_reg(result);
        auto from = to_asm_operand(c, v);
        const bool use_lea = from.is_reg() && !from.is_phys(to);
        switch (type) {
        case imm_i64:
          if (use_lea) {
            c->emit_lea(to, a::mem64(from.reg(), from.reg()), with_metadata(mdi::comment("Tag u64"), mdi::copy(o->metadata())));
          } else {
            c->emit_shl(to, from, a::imm8(1), with_metadata(mdi::comment("Tag u64"), mdi::copy(o->metadata())));
          }
          break;
        case imm_fn_ref:
          emit_tag_add(c, o, to, from, use_lea, tag_closure_ref, "Tag func");
          break;
        case imm_closure_ref:
          emit_tag_add(c, o, to, from, use_lea, tag_closure_ref, "Tag closure");
          break;
        case imm_vec_ref:
          emit_tag_add(c, o, to, from, use_lea, tag_vec_ref, "Tag vec");
          break;
        case imm_str_ref:
          emit_tag_add(c, o, to, from, use_lea, tag_str_ref, "Tag str");
          break;
        case imm_rec_ref:
        case imm_f32:
          J_TODO();
        case imm_act_rec: J_ASSERT_FAIL("ActRec tagging not supported.");
        case imm_sym_id: J_ASSERT_FAIL("Sym tagging not supported.");
        case imm_bool: J_ASSERT_FAIL("Bool tagging not supported.");
        case imm_nil: J_ASSERT_FAIL("Nil tagging not supported.");
        case imm_range: J_ASSERT_FAIL("Range tagging not supported.");
        case imm_undefined: J_FAIL("Undefined in tag");
        }
      }

      void operator()(value_tag<op_type::untag>, const c::op * o, cir::loc result, imm_type type, const cir::input & v) {
        auto to = to_asm_reg(result);
        auto from = to_asm_operand(c, v);
        const bool use_lea = from.is_reg() && !from.is_phys(to);
        switch (type) {
        case imm_i64:
          c->emit_sar(to, from, a::imm8(1), with_metadata(mdi::comment("Untag i64"), mdi::copy(o->metadata())));
          break;
        case imm_fn_ref:
          emit_untag_subtract(c, o, to, from, use_lea, tag_fn_ref, "Untag func");
          break;
        case imm_closure_ref:
          emit_untag_subtract(c, o, to, from, use_lea, tag_closure_ref, "Untag closure");
          break;
        case imm_vec_ref:
          emit_untag_subtract(c, o, to, from, use_lea, tag_vec_ref, "Untag vec");
          break;
        case imm_str_ref:
          emit_untag_subtract(c, o, to, from, use_lea, tag_str_ref, "Untag str");
          break;
        case imm_rec_ref:
        case imm_f32:
          J_TODO();
        case imm_act_rec: J_ASSERT_FAIL("ActRec untagging not supported.");
        case imm_sym_id: J_ASSERT_FAIL("Sym untagging not supported.");
        case imm_bool: J_ASSERT_FAIL("Bool untagging not supported.");
        case imm_nil: J_ASSERT_FAIL("Nil untagging not supported.");
        case imm_range: J_ASSERT_FAIL("Range untagging not supported.");
        case imm_undefined: J_FAIL("Undefined in untag");
        }
      }

    void operator()(value_tag<op_type::jmpc>, const c::op * o, cir::condition cond, const cir::input & lhs, const cir::input & rhs,
                    const cir::bb * then_bb, const cir::bb * else_bb) {
      if (then_bb != else_bb) {
        auto lhs_op = to_asm_operand(c, lhs);
        auto rhs_op = to_asm_operand(c, rhs);
        if (then_bb == next_bb) {
          then_bb = else_bb;
          else_bb = next_bb;
          cond = negate_condition(cond);
        }
        switch (cond) {
        case cir::condition::eq:
          if (lhs_op == rhs_op) {
            c->emit_jmp(then_bb->asm_label, with_metadata(mdi::comment("Trivially true eq"), mdi::copy(o->metadata())));
            return;
          }
          c->emit_cmp_jmp(a::cmp_flag_condition::eq, lhs_op, rhs_op, then_bb->asm_label, copy_metadata(o->metadata()));
          break;
        case cir::condition::neq:
          if (lhs_op != rhs_op) {
            c->emit_cmp_jmp(a::cmp_flag_condition::neq, lhs_op, rhs_op, then_bb->asm_label, copy_metadata(o->metadata()));
          }
          break;
        default:
          J_TODO();
        }
      }
      if (else_bb != next_bb) {
        c->emit_jmp(else_bb->asm_label, with_metadata(mdi::comment("Else"), mdi::copy(o->metadata())));
      }
    }

      void operator()(value_tag<op_type::jmpt>, const c::op * o, const cir::input & v, const cir::bb * J_NOT_NULL then_bb, const cir::bb * J_NOT_NULL else_bb) {
        const bool negate = then_bb == next_bb;
        c->emit_cmp_jmp(a::cmp_flag_condition::eq,
                        v.op_result_data.use.loc_out.as_gpr8(),
                        a::imm8((u8_t)tag_nil),
                        else_bb->asm_label,
                        with_metadata(mdi::comment(negate ? "(Negated)" : "Jump if nil"), mdi::copy(o->metadata())));
        c->emit_cmp_jmp((negate ? a::cmp_flag_condition::neq : a::cmp_flag_condition::eq),
                        v.op_result_data.use.loc_out.as_gpr8(),
                        a::imm8((u8_t)true_v),
                        (negate ? else_bb->asm_label : then_bb->asm_label),
                        with_metadata(mdi::comment(negate ? "Jump if not false" : "Jump if false"), mdi::copy(o->metadata())));
        if (!negate && then_bb != next_bb) {
          c->emit_jmp(then_bb->asm_label, copy_metadata(o->metadata()));
        }
      }

      void operator()(value_tag<op_type::jmptype>, const c::op * o,
                      const cir::input & v,
                      imm_type_mask types,
                      cir::loc temp_loc,
                      const cir::bb * J_NOT_NULL then_bb,
                      const cir::bb * J_NOT_NULL else_bb) {
        J_ASSERT(types);
        auto in = v.op_result_data.use.loc_out.as_gpr8();
        c->emit_test_jmp(types.has(imm_i64) ? a::test_flag_condition::nz : a::test_flag_condition::z, in, a::imm8(1),
                         else_bb->asm_label,
                         with_metadata(mdi::comment(types.has(imm_i64) ? "Accept i64" : "Reject i64"), mdi::copy(o->metadata())));
        types.clear(imm_i64);
        if (types) {
          auto temp = temp_loc.as_gpr64();
          u64_t type_mask = 0U;
          for (auto t : types) {
            u8_t index = (u8_t)imm_tag_by_type(t);
            J_ASSERT(index < 64);
            type_mask |= 1ULL << index;
            if (index < 8) {
              type_mask |= 1ULL << (index + 4);
              type_mask |= 1ULL << (index + 8);
              type_mask |= 1ULL << (index + 12);
              type_mask |= 1ULL << (index + 16);
              type_mask |= 1ULL << (index + 20);
              type_mask |= 1ULL << (index + 24);
              type_mask |= 1ULL << (index + 28);
              type_mask |= 1ULL << (index + 32);
              type_mask |= 1ULL << (index + 36);
              type_mask |= 1ULL << (index + 40);
              type_mask |= 1ULL << (index + 44);
              type_mask |= 1ULL << (index + 48);
              type_mask |= 1ULL << (index + 52);
              type_mask |= 1ULL << (index + 56);
              type_mask |= 1ULL << (index + 60);
            }
          }
          J_ASSERT(type_mask);
          auto formatted = strings::format("{}", types);
          c->emit_mov(temp, a::imm64(type_mask), with_metadata(mdi::comment(formatted), mdi::copy(o->metadata())));
          c->emit_bit_test_jmp(temp, in, else_bb->asm_label, true, /*map_arg0_to_1.apply(o->metadata())*/copy_metadata(o->metadata()));
        }

        if (next_bb != then_bb) {
          c->emit_jmp(then_bb->asm_label, copy_metadata(o->metadata()));
        }
      }

      a::operand to_branch_operand(const cir::input & oper) const noexcept {
        switch (oper.type) {
        case cir::input_type::none:
          J_UNREACHABLE();
        case cir::input_type::mem:
          J_TODO();
        case cir::input_type::op_result:
          if (oper.op_result_data.use.loc_out.type() == cir::loc_type::spill_slot) {
            return c->spill_slot_operand(oper.op_result_data.use.loc_in.spill_slot_index());
          }
          return to_asm_reg(oper.op_result_data.use.loc_in);
        case cir::input_type::constant:
          J_FAIL("Constant branch not allowed");
        case cir::input_type::reloc:
          J_ASSERT(oper.reloc_data.reloc_type != cir::reloc_type::var_addr);
          return a::imm32(a::reloc_source{
              a::reloc_source_type::fn_addr,
              oper.reloc_data.calling_convention,
              { oper.reloc_data.id }},
            oper.reloc_data.addend);
        }
      }


      J_ALWAYS_INLINE constexpr void operator()(value_tag<op_type::phi>, const c::op *, cir::loc, span<const cir::input>) noexcept { }
    };

    a::function cir_to_mc_cc(compilation::context & ctx, compilation::const_table & constants, strings::const_string_view pkg_name, strings::const_string_view name, a::target & tgt, cir::cir_ssa * J_NOT_NULL ssa) {
      a::function_builder builder(&ctx.temp_pool, pkg_name, name, constants);
      auto compiler = tgt.create_compiler(&builder);

      visitor v{compiler.get(), ssa, nullptr, nullptr, tgt.get_reg_info()};
      for (auto it = ssa->in_reverse_postorder().begin(), end = ssa->in_reverse_postorder().end(); it != end; ++it) {
        auto next = it;
        ++next;
        v.next_bb = next == end ? nullptr : &*next;

        for (auto exit : it->exit_edges()) {
          auto exit_bb = exit.bb;
          if (exit_bb->asm_label) {
            continue;
          }
          exit_bb->asm_label = compiler->allocate_label(exit_bb->name);
        }

        if (it->asm_label && !it->asm_label->is_valid) {
          compiler->emit_label(it->asm_label);
        }

        v.bb = &*it;
        for (auto & op : *it) {
          cir::visit_cir_op(&op, v);
        }
      }
      return builder.build(&ctx.metadata_pool());
    }
  }

  [[nodiscard]] mc_functions * cir_to_mc(compilation::context & ctx, cir::cir_function * J_NOT_NULL cir_fn) {
    a::amd64::abi::ia64_target tgt;
    u8_t pkg_index = ctx.pkg.symbol_table.names.package_id;
    strings::const_string_view pkg_name = ctx.name(id(package_id_packages, pkg_index));

    mc_functions * result = (mc_functions*)ctx.temp_pool.allocate(
      sizeof(mc_functions) + sizeof(mc_closure) * (cir_fn->num_closures));
    ::new (result) mc_functions{{}, (i32_t)cir_fn->closures().size()};

    auto cur_closure = result->closures().begin();
    for (auto & closure : cir_fn->closures()) {
      (::new (cur_closure++) mc_closure{closure.symbol, {}})->asm_funcs[functions::calling_convention::full_call]
        = cir_to_mc_cc(ctx, closure.constants, pkg_name, ctx.name(closure.symbol), tgt, &closure.ssa_full_call);
    }



  result->asm_funcs[functions::calling_convention::full_call] = cir_to_mc_cc(ctx, ctx.constants, pkg_name, ctx.name(), tgt, &cir_fn->ssa_full_call);
  result->asm_funcs[functions::calling_convention::abi_call] = cir_to_mc_cc(ctx, ctx.constants, pkg_name, ctx.name(), tgt, &cir_fn->ssa_abi_call);

  ctx.func_info->cc_info[functions::calling_convention::full_call].asm_info = result->asm_funcs[functions::calling_convention::full_call].info;
  ctx.func_info->cc_info[functions::calling_convention::abi_call].asm_info = result->asm_funcs[functions::calling_convention::abi_call].info;
  return result;
  }
}
