#pragma once

#include "lisp/assembly/flag_condition.hpp"
#include "lisp/assembly/operand.hpp"
#include "lisp/assembly/registers.hpp"
#include "lisp/assembly/functions/function_builder.hpp"
#include "lisp/common/metadata_init.hpp"

namespace j::lisp::assembly {
  enum class integer_binop : u8_t {
    add = 0U,
    sub,

    smul,
    sdiv,
    umul,
    udiv,

    band,
    bor,
    bxor,

    bshl,
    bshr,
    bsar,
  };

  enum class integer_unop : u8_t {
    neg = 0U,
    bnot,
  };

  struct compiler {
    compiler(function_builder * J_NOT_NULL builder) noexcept;

    virtual ~compiler();

    J_BOILERPLATE(compiler, COPY_DEL)

    [[nodiscard]] virtual operand spill_slot_operand(u32_t i) const noexcept = 0;

    virtual void emit_mov(operand to, operand from, const metadata_init & mds = {}) = 0;
    virtual void emit_swap(reg a, reg b, const metadata_init & mds = {}) = 0;
    virtual void emit_push(operand r, const metadata_init & mds = {}) = 0;
    virtual void emit_pop(operand r, const metadata_init & mds = {}) = 0;

    virtual void emit_enter_stack_frame(u32_t spill_slots, const metadata_init & mds = {}) = 0;
    virtual void emit_reserve_stack_space(u32_t bytes, const metadata_init & mds = {}) = 0;
    virtual void emit_leave_stack_frame(u32_t spill_slots, const metadata_init & mds = {}) = 0;
    virtual void emit_return(const metadata_init & mds = {}) = 0;

    virtual void emit_trap(trap_type trap, const metadata_init & mds = {}) = 0;

    // virtual void emit_mul_add(reg to, operand value, u64_t scale, u64_t displacement) = 0;

    [[nodiscard]] inline J_RETURNS_NONNULL label_record * allocate_label(const strings::const_string_view & name) {
      return builder->allocate_label(name);
    }

    template<typename... Args>
    void emit_type_check_info(imm_type_mask expected_type, Args && ... args) {
      builder->push_type_check_info(expected_type, static_cast<Args &&>(args)...);
    }

    inline void emit_label(label_record * J_NOT_NULL label) {
      builder->push_label(label);
    }

    virtual void emit_alloc_full_call(operand num_args, const metadata_init & mds = {}) = 0;
    virtual void emit_dealloc_full_call(operand num_args, const metadata_init & mds = {}) = 0;
    virtual void emit_load_sp(reg to, const metadata_init & mds = {}) = 0;
    virtual void emit_write_full_call_arg(u32_t index, operand op, const metadata_init & mds = {}) = 0;

    virtual void emit_call(operand fn, const metadata_init & mds = {}) = 0;

    virtual void emit_jmp(label_record * J_NOT_NULL label, const metadata_init & mds = {}) = 0;
    virtual void emit_test_jmp(test_flag_condition cond, operand lhs, operand rhs, label_record * J_NOT_NULL label,
                               const metadata_init & mds = {}) = 0;
    virtual void emit_cmp_jmp(cmp_flag_condition cond, operand lhs, operand rhs, label_record * J_NOT_NULL label,
                              const metadata_init & mds = {}) = 0;
    virtual void emit_test_mov(test_flag_condition cond, reg to, operand lhs, operand rhs, operand from,
                              const metadata_init & mds = {}) = 0;
    virtual void emit_cmp_mov(cmp_flag_condition cond, reg to, operand lhs, operand rhs, operand from,
                              const metadata_init & mds = {}) = 0;

    virtual void emit_bit_test_jmp(operand mask, operand bit, label_record * J_NOT_NULL label, bool negate,
                                   const metadata_init & mds = {}) = 0;

    virtual void emit_cmp_get(cmp_flag_condition cond, reg to, operand lhs, operand rhs,
                              const metadata_init & mds = {}) = 0;

    virtual void emit_lea(reg to, operand op, const metadata_init & mds = {}) = 0;

    virtual void emit_op(integer_binop op, reg to, operand lhs, operand rhs, const metadata_init & mds = {}) = 0;
    virtual void emit_op(integer_unop op, reg to, operand operand, const metadata_init & mds = {}) = 0;

    J_ALWAYS_INLINE_NO_DEBUG void emit_neg(reg to, const operand & value, const metadata_init & mds = {}) {
      emit_op(integer_unop::neg, to, value, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_not(reg to, const operand & value, const metadata_init & mds = {}) {
      emit_op(integer_unop::bnot, to, value, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_iadd(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::add, to, lhs, rhs, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_isub(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::sub, to, lhs, rhs, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_or(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::bor, to, lhs, rhs, mds);
    }
    J_ALWAYS_INLINE_NO_DEBUG void emit_and(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::band, to, lhs, rhs, mds);
    }
    J_ALWAYS_INLINE_NO_DEBUG void emit_xor(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::bxor, to, lhs, rhs, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_shr(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::bshr, to, lhs, rhs, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_sar(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::bsar, to, lhs, rhs, mds);
    }

    J_ALWAYS_INLINE_NO_DEBUG void emit_shl(reg to, const operand & lhs, const operand & rhs, const metadata_init & mds = {}) {
      emit_op(integer_binop::bshl, to, lhs, rhs, mds);
    }

    virtual void emit_copy_qwords(reg to, reg from, reg count, const metadata_init & mds = {}) = 0;

    function_builder * builder = nullptr;
  };
}
