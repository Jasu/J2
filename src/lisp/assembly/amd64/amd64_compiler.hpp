#include "lisp/assembly/compiler.hpp"

namespace j::lisp::assembly::amd64 {
  class amd64_compiler final : public compiler {
  public:
    using compiler::compiler;

    [[nodiscard]] operand spill_slot_operand(u32_t i) const noexcept override;

    void emit_mov(operand to, operand from, const metadata_init & mds = {}) override;
    void emit_swap(reg a, reg b, const metadata_init & mds = {}) override;
    void emit_push(operand r, const metadata_init & mds = {}) override;
    void emit_pop(operand r, const metadata_init & mds = {}) override;

    void emit_cmp_get(cmp_flag_condition cond, reg to, operand lhs, operand rhs,
                      const metadata_init & mds = {}) override;

    void emit_enter_stack_frame(u32_t spill_slots, const metadata_init & mds = {}) override;
    void emit_reserve_stack_space(u32_t bytes, const metadata_init & mds = {}) override;
    void emit_leave_stack_frame(u32_t spill_slots, const metadata_init & mds = {}) override;
    void emit_return(const metadata_init & mds = {}) override;

    void emit_alloc_full_call(operand num_args, const metadata_init & mds = {}) override;
    void emit_dealloc_full_call(operand num_args, const metadata_init & mds = {}) override;
    void emit_write_full_call_arg(u32_t index, operand op, const metadata_init & mds = {}) override;
    void emit_load_sp(reg to, const metadata_init & mds = {}) override;

    void emit_call(operand fn, const metadata_init & mds = {}) override;

    void emit_jmp(label_record * J_NOT_NULL label, const metadata_init & mds = {}) override;
    void emit_test_jmp(test_flag_condition cond, operand lhs, operand rhs, label_record * J_NOT_NULL label,
                       const metadata_init & mds = {}) override;
    void emit_cmp_jmp(cmp_flag_condition cond, operand lhs, operand rhs,  label_record * J_NOT_NULL label,
                       const metadata_init & mds = {}) override;
    void emit_test_mov(test_flag_condition cond, reg to, operand lhs, operand rhs, operand from,
                       const metadata_init & mds = {}) override;
    void emit_cmp_mov(cmp_flag_condition cond, reg to, operand lhs, operand rhs, operand from,
                      const metadata_init & mds = {}) override;
    void emit_bit_test_jmp(operand mask, operand bit, label_record * J_NOT_NULL label, bool negate,
                       const metadata_init & mds = {}) override;

    void emit_op(integer_binop op, reg to, operand lhs, operand rhs, const metadata_init & mds = {}) override;
    void emit_op(integer_unop op, reg to, operand value, const metadata_init & mds = {}) override;

    void emit_lea(reg to, operand op, const metadata_init & mds = {}) override;
    void emit_copy_qwords(reg to, reg from, reg count, const metadata_init & mds = {}) override;

    void emit_trap(trap_type type, const metadata_init & mds = {}) override;
  };
}
