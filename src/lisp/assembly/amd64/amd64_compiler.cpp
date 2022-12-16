#include "lisp/assembly/amd64/amd64_compiler.hpp"
#include "lisp/assembly/amd64/emitter.hpp"
#include "lisp/assembly/amd64/emit_conditional.hpp"
#include "lisp/common/metadata_init.hpp"
#include "lisp/assembly/amd64/inst.hpp"
#include "lisp/assembly/amd64/emit_mov.hpp"

namespace j::lisp::assembly::amd64 {
  namespace i = inst;
  using mdk = metadata_key;
  using mdm = metadata_mapping;

  namespace {
    constexpr inline metadata_mapping map_loc{metadata_map()};
    constexpr inline metadata_mapping map_unop{metadata_map(arg_from_result(0), arg_from(1, 0))};
    constexpr inline metadata_mapping map_result_only{metadata_map(arg_from_result(0))};
    constexpr inline metadata_mapping map_call{metadata_map(result_from_result, args_from(0))};
    constexpr inline metadata_mapping map_cmp_nores{metadata_map(arg_from(0, 0), arg_from(1, 1))};
    constexpr inline metadata_mapping map_cmp_res{metadata_map(result_from_result, arg_from(0, 0), arg_from(1, 1))};
    constexpr inline metadata_mapping map_cmov{metadata_map(result_from_result, arg_from_result(0), arg_from(1, 3))};
    constexpr inline metadata_mapping map_binop{metadata_map(arg_from_result(0), arg_from(1, 0), arg_from(2, 1))};
    constexpr inline metadata_mapping map_binop_flipped{metadata_map(arg_from_result(0), arg_from(1, 1), arg_from(2, 0))};

    bool emit_two_op(function_builder & buf, const instruction_encodings & enc, reg to, operand lhs, operand rhs, const metadata_init & mds, bool match_size = true) {
      if (lhs.is_phys(to)) {
        if (match_size) {
          if (rhs.is_reg()) {
            rhs = rhs.with_width(to.width());
          } else if (!rhs.is_imm()) {
            to = to.with_width(rhs.width());
          }
        }
      } else if ((rhs.is_phys(to))) {
        return false;
      } else {
        buf.push_addr_info(map_loc.apply(mds));
        emit_mov(buf, to, lhs);
      }
      buf.push_addr_info(map_binop.apply(mds));
      emit(buf, enc, to, rhs);
      return true;
    }

    void emit_two_op_commutative(function_builder & buf, const instruction_encodings & enc, reg to, operand lhs, operand rhs, const metadata_init & mds, bool match_size = true) {
      if (!emit_two_op(buf, enc, to, lhs, rhs, mds, match_size)) {
        buf.push_addr_info(map_binop_flipped.apply(mds));
        emit(buf, enc, to, lhs);
      }
    }
  }

  void amd64_compiler::emit_mov(operand to, operand from, const metadata_init & mds) {
    builder->push_addr_info(map_unop.apply(mds));
    amd64::emit_mov(*builder, to, from);
  }
  void amd64_compiler::emit_swap(reg to, reg from, const metadata_init & mds) {
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::xchg, to, from);
  }

  [[nodiscard]] operand amd64_compiler::spill_slot_operand(u32_t i) const noexcept {
    return mem64(rbp, -(1 + i) * 8);
  }

  void amd64_compiler::emit_push(operand r, const metadata_init & mds) {
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::push, r);
  }
  void amd64_compiler::emit_pop(operand r, const metadata_init & mds) {
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::pop, r);
  }


  void amd64_compiler::emit_cmp_get(cmp_flag_condition cond, reg to, operand lhs, operand rhs,
                                    const metadata_init & mds) {
    const bool aliases = lhs.is_phys(to) || rhs.is_phys(to);
    if (!aliases) {
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::bxor, to.with_width(mem_width::dword), to.with_width(mem_width::dword));
    }
    builder->push_addr_info(map_cmp_res.apply(mds));
    emit(*builder, i::cmp, lhs, rhs);
    builder->push_addr_info(map_result_only.apply(mds));
    emit_setcc(*builder, cond, to.with_width(mem_width::byte));
    if (aliases) {
      builder->push_addr_info(map_result_only.apply(mds));
      emit(*builder, i::movzx, to.with_width(mem_width::dword), to.with_width(mem_width::byte));
    }
  }

  void amd64_compiler::emit_enter_stack_frame(u32_t spill_slots, const metadata_init & mds) {
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::push, rbp);
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::mov, rbp, rsp);
    if (spill_slots) {
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::sub, rsp, imm32(align_up(spill_slots * 8U, 16U)));
    }
  }

  void amd64_compiler::emit_reserve_stack_space(u32_t sz, const metadata_init & mds) {
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::sub, rsp, imm32(align_up(sz, 16)));
  }

  void amd64_compiler::emit_leave_stack_frame(u32_t sz, const metadata_init & mds) {
    if (sz) {
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::mov, rsp, rbp);
    }
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::pop, rbp);
  }

  void amd64_compiler::emit_return(const metadata_init & mds) {
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::ret);
  }

  void amd64_compiler::emit_alloc_full_call(operand num_args, const metadata_init & mds) {
    if (num_args.is_imm()) {
      auto immval = align_up(num_args.imm() * 8, 16);
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::sub, rsp, imm((u8_t)immval < 128 ? mem_width::byte : mem_width::dword, immval));
    } else {
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::add, num_args, imm8(1U));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::shl, num_args, imm8(3U));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::sub, rsp, num_args);
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::band, rsp, imm64(~0xFULL));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::shr, num_args, imm8(3U));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::sub, num_args, imm8(1U));
    }
  }

  void amd64_compiler::emit_dealloc_full_call(operand num_args, const metadata_init & mds) {
    if (num_args.is_imm()) {
      auto immval = align_up(num_args.imm() * 8, 16);
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::add, rsp, imm((u8_t)immval < 128 ? mem_width::byte : mem_width::dword, immval));
    } else {
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::add, num_args, imm32(1U));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::shl, num_args, imm32(3U));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::add, rsp, num_args);
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::band, rsp, imm64(~0xFULL));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::shr, num_args, imm32(3U));
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::sub, num_args, imm32(1U));
    }
  }
  void amd64_compiler::emit_write_full_call_arg(u32_t index, operand op, const metadata_init & mds) {
    builder->push_addr_info(map_unop.apply(mds));
    emit(*builder, i::mov, mem64(rsp, index * 8U), op.is_imm() ? op.with_width(mem_width::dword) : op.with_width(mem_width::qword));
  }

  void amd64_compiler::emit_load_sp(reg to, const metadata_init & mds) {
    J_ASSERT(to.width() == mem_width::qword && to != rsp);
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::mov, to, rsp);
  }

  void amd64_compiler::emit_call(operand fn, const metadata_init & mds) {
    builder->push_addr_info(map_call.apply(mds));
    emit(*builder, i::call, fn);
  }

  void amd64_compiler::emit_jmp(label_record * J_NOT_NULL label, const metadata_init & mds) {
    metadata_init md = mds;
    md.args[0].name = label->get_label();
    builder->push_addr_info(md);
    emit_reljmp(*builder, i::jmp, label);
  }

  void amd64_compiler::emit_test_jmp(test_flag_condition cond, operand lhs, operand rhs, label_record * J_NOT_NULL label,
                                     const metadata_init & mds) {
    if (lhs.is_imm()) {
      operand temp = lhs;
      lhs = rhs;
      rhs = temp;
    }
    builder->push_addr_info(map_cmp_nores.apply(mds));
    emit(*builder, i::test, lhs, rhs);
    metadata_init md = mds;
    md.args[0].name = label->get_label();
    builder->push_addr_info(md);
    emit_conditional_branch(*builder, cond, label);
  }

  void amd64_compiler::emit_cmp_jmp(cmp_flag_condition cond, operand lhs, operand rhs, label_record * J_NOT_NULL label,
                                    const metadata_init & mds) {
    if (lhs.is_imm()) {
      operand temp = lhs;
      lhs = rhs;
      rhs = temp;
      cond = reverse_condition(cond);
    }
    builder->push_addr_info(map_cmp_nores.apply(mds));
    emit(*builder, i::cmp, lhs, rhs);
    metadata_init md = mds;
    md.args[0].name = label->get_label();
    builder->push_addr_info(md);
    emit_conditional_branch(*builder, cond, label);
  }

  void amd64_compiler::emit_test_mov(test_flag_condition cond, reg to, operand lhs, operand rhs, operand from,
                                     const metadata_init & mds) {
    if (lhs.is_imm()) {
      operand temp = lhs;
      lhs = rhs;
      rhs = temp;
    }
    builder->push_addr_info(map_cmp_res.apply(mds));
    emit(*builder, i::test, lhs, rhs);
    builder->push_addr_info(map_cmov.apply(mds));
    emit_conditional_move(*builder, cond, to, from);
  }

  void amd64_compiler::emit_cmp_mov(cmp_flag_condition cond, reg to, operand lhs, operand rhs, operand from,
                                     const metadata_init & mds) {
    if (lhs.is_imm()) {
      operand temp = lhs;
      lhs = rhs;
      rhs = temp;
      cond = reverse_condition(cond);
    }
    builder->push_addr_info(map_cmp_res.apply(mds));
    emit(*builder, i::cmp, lhs, rhs);
    builder->push_addr_info(map_cmov.apply(mds));
    emit_conditional_move(*builder, cond, to, from);
  }

  void amd64_compiler::emit_bit_test_jmp(operand mask, operand bit, label_record * J_NOT_NULL label, bool negate,
                                     const metadata_init & mds) {
    builder->push_addr_info(map_cmp_nores.apply(mds));
    emit(*builder, i::bt, mask, bit.with_width(mask.width()));
    metadata_init md = mds;
    md.args[0].name = label->get_label();
    builder->push_addr_info(md);
    emit_reljmp(*builder, negate ? i::ja : i::jb, label);
  }

  void amd64_compiler::emit_trap(trap_type type, const metadata_init & mds) {
    builder->push_addr_info(mds);
    emit(*builder, type == trap_type::debug ? i::int3 : i::into);
  }

  void amd64_compiler::emit_lea(reg to, operand op, const metadata_init & mds) {
    J_ASSERT(op.is_mem());
    reg base = op.base();
    reg index = op.index();
    auto disp = op.disp();
    if (to.phys() == base.phys() && !index) {
      if (!disp) {
        return;
      }
      builder->push_addr_info(map_unop.apply(mds));
      emit(*builder, i::add, to, ((u8_t)disp < 128 || disp >= I8_MIN) ? imm8(disp) : imm32(disp));
    } else {
      builder->push_addr_info(map_unop.apply(mds));
      emit(*builder, i::lea, to, op);
    }
  }

  void amd64_compiler::emit_copy_qwords(reg to, reg from, reg count, const metadata_init & mds) {
    J_ASSERT(to == rdi && from == rsi && count == ecx);
    builder->push_addr_info(map_loc.apply(mds));
    emit_rep(*builder);
    builder->push_addr_info(map_loc.apply(mds));
    emit(*builder, i::movsq, to, from);
  }

  void amd64_compiler::emit_op(integer_binop op, reg to, operand lhs, operand rhs, const metadata_init & mds) {
    const bool is_rhs_imm = rhs.is_imm(), is_lhs_imm = lhs.is_imm();
    operand imm_operand = is_rhs_imm ? rhs : is_lhs_imm ? lhs : operand{};
    operand non_imm_operand = is_rhs_imm ? lhs : is_lhs_imm ? rhs : operand{};
    switch (op) {
    case integer_binop::sub:
      if (is_rhs_imm && lhs.is_reg() && !lhs.is_phys(to)) {
        emit_lea(to, mem64(lhs.reg(), -rhs.imm()), mds);
      } else if (!emit_two_op(*builder, i::sub, to, lhs, rhs, mds)) {
        builder->push_addr_info(map_binop_flipped.apply(mds));
        emit(*builder, i::sub, to, lhs);
        builder->push_addr_info(map_result_only.apply(mds));
        emit(*builder, i::neg, to);
      }
      break;
    case integer_binop::add:
      if (rhs.is_reg() && lhs.is_reg() && !lhs.is_phys(to) && !rhs.is_phys(to)) {
        emit_lea(to, mem64(lhs.reg(), rhs.reg()), mds);
      } else if (imm_operand && non_imm_operand.is_reg() && !non_imm_operand.is_phys(to)) {
        emit_lea(to, mem64(non_imm_operand.reg(), imm_operand.imm()), mds);
      } else {
        emit_two_op_commutative(*builder, i::add, to, lhs, rhs, mds, true);
      }
      break;
    case integer_binop::umul:
    case integer_binop::smul:
      emit_two_op_commutative(*builder, op == integer_binop::umul ? (const instruction_encodings &)i::mul : i::imul, to, lhs, rhs, mds);
      break;
    case integer_binop::udiv: {
      J_ASSERT(to == lhs.reg() && to.is(preg::rax));
      reg rem = rdx.with_width(to.width());
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, i::bxor, rem, rem);
      builder->push_addr_info(map_binop.apply(mds));
    emit(*builder, i::div, rem, to, rhs);
    break;
    }
    case integer_binop::sdiv: {
      J_ASSERT(to == lhs.reg() && to.is(preg::rax));
      instruction_encodings fixup_encs;
      switch (to.width()) {
      case mem_width::byte:
        builder->push_addr_info(map_binop.apply(mds));
        emit(*builder, i::idiv, to, rhs);
        return;
      case mem_width::word: fixup_encs = i::cwd; break;
      case mem_width::dword: fixup_encs = i::cdq; break;
      case mem_width::qword: fixup_encs = i::cqo; break;

      case mem_width::vec128: case mem_width::vec256:
      case mem_width::vec512: case mem_width::none:
        J_FAIL("Invalid idiv reg width");
      }
      reg rem = rdx.with_width(to.width());
      builder->push_addr_info(map_loc.apply(mds));
      emit(*builder, fixup_encs, rem, to);
      builder->push_addr_info(map_binop.apply(mds));
      emit(*builder, i::idiv, rem, to, rhs);
      break;
    }

    case integer_binop::bshl:
      if (is_rhs_imm && rhs.imm() == 1 && lhs.is_reg() && !lhs.is_phys(to)) {
        if (lhs.width() == mem_width::dword) {
          to.set_width(mem_width::dword);
        }
        builder->push_addr_info(map_unop.apply(mds));
        emit(*builder, i::lea, to, mem64(lhs.reg(), lhs.reg()));
      } else if (!emit_two_op(*builder, i::shl, to, lhs, rhs.with_width(mem_width::byte), mds, false)) {
        J_TODO();
      }
      break;

    case integer_binop::band:
      if (imm_operand && imm_operand.imm() == U32_MAX) {
        builder->push_addr_info(map_unop.apply(mds));
        emit(*builder, i::mov, to.with_width(mem_width::dword), non_imm_operand.with_width(mem_width::dword));
      } else if (imm_operand && imm_operand.imm() == 0xFF) {
        builder->push_addr_info(map_unop.apply(mds));
        emit(*builder, i::movzx, to.with_width(mem_width::dword), non_imm_operand.with_width(mem_width::byte));
      } else {
        emit_two_op_commutative(*builder, i::band, to, lhs, rhs, mds);
      }
      break;
    case integer_binop::bor:
    case integer_binop::bxor:
      emit_two_op_commutative(*builder, op == integer_binop::bxor ? i::bxor : i::bor, to, lhs, rhs, mds);
      break;
    case integer_binop::bsar:
    case integer_binop::bshr:
      if (!emit_two_op(*builder, op == integer_binop::bsar ? i::sar : i::shr, to, lhs, rhs.with_width(mem_width::byte), mds, false)) {
        J_TODO();
      }
      break;
    }
  }

  void amd64_compiler::emit_op(integer_unop op, reg to, operand value, const metadata_init & mds) {
    if (!value.is_phys(to)) {
      if (op == integer_unop::neg) {
        builder->push_addr_info(map_result_only.apply(mds));
        const auto to_dword = to.with_width(mem_width::dword);
        emit(*builder, i::bxor, to_dword, to_dword);
        builder->push_addr_info(map_unop.apply(mds));
        emit(*builder, i::sub, to, value);
        return;
      }
      emit_mov(to, value, map_result_only.apply(mds));
    }
    builder->push_addr_info(map_unop.apply(mds));
    emit(*builder, op == integer_unop::neg ? i::neg : i::bnot, to);
  }
}
