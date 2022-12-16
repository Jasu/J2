#pragma once

#include "lisp/air/exprs/expr_type.hpp"

namespace j::lisp::air::exprs {
  /// How representations of the result and the inputs interrelate.
  enum class rep_spec_mode : u8_t {
    /// Changing the representation of the input has no effect of the representation of the result.
    constant,
    /// Result representation is independent of input representations, but all inputs must have the same representation.
    ///
    /// E.g.:
    ///   - Division of tagged or untagged integers always yields an untagged integer.
    ///     (note that `(/ tagged untagged)` could be implemented as a special case)
    ///   - `eq` and `neq` comparisons always yield a boolean, but the inputs have to match.
    inputs_same,
    /// Result has the same representation as all inputs.
    ///
    /// E.g.:
    ///   - Addition and subtraction of tagged or untagged integers yields the same representation.
    ///   - Simply copying the input has the same representation.
    all_same_rep,
    /// Result has the same type and representation as all inputs.
    all_same_rep_and_type,
    /// Result has the same representation as the last input.
    progn,
    /// Result has the same representation as the results of its branches.
    branch_result,
    /// Result has the same representation as the first argument.
    in0,
    /// Result has the same representation as the second argument.
    in1,
    /// Result has the same representation as the third argument.
    in2,
    /// Result has the same type as the first argument.
    type_in0,
    /// Result has the same type as the second argument.
    type_in1,
    /// Result has the same type as the third argument.
    type_in2,
    /// Result is bound to the representation of a lexical variable.
    lexvar,
  };

#define J_REP_SPEC_MODE(EXPR, MODE) [(u8_t)expr_type::EXPR] = rep_spec_mode::MODE

  constexpr inline rep_spec_mode rep_spec_modes[num_expr_types_v]{
    J_REP_SPEC_MODE(nop, constant),

    J_REP_SPEC_MODE(fn_body, progn),
    J_REP_SPEC_MODE(fn_arg, constant),
    J_REP_SPEC_MODE(fn_rest_arg, constant),
    J_REP_SPEC_MODE(fn_sc_arg, constant),

    J_REP_SPEC_MODE(lex_rd, lexvar),
    J_REP_SPEC_MODE(lex_wr, in0),
    J_REP_SPEC_MODE(lex_rd_bound, lexvar),
    J_REP_SPEC_MODE(lex_wr_bound, in0),
    J_REP_SPEC_MODE(let, progn),

    J_REP_SPEC_MODE(sym_val_rd, constant),
    J_REP_SPEC_MODE(sym_val_wr, in0),

    J_REP_SPEC_MODE(ld_const, constant),

    J_REP_SPEC_MODE(ineg, in0),
    J_REP_SPEC_MODE(iadd, all_same_rep),
    J_REP_SPEC_MODE(isub, all_same_rep),
    J_REP_SPEC_MODE(imul, constant),
    J_REP_SPEC_MODE(idiv, constant),

    J_REP_SPEC_MODE(type_assert, constant),
    J_REP_SPEC_MODE(dyn_type_assert, constant),

    J_REP_SPEC_MODE(b_if, branch_result),
    J_REP_SPEC_MODE(do_until, branch_result),

    J_REP_SPEC_MODE(lor, branch_result),
    J_REP_SPEC_MODE(land, branch_result),
    J_REP_SPEC_MODE(lnot, constant),

    J_REP_SPEC_MODE(to_bool, constant),

    J_REP_SPEC_MODE(eq, inputs_same),
    J_REP_SPEC_MODE(neq, inputs_same),

    J_REP_SPEC_MODE(tag, type_in0),
    J_REP_SPEC_MODE(untag, type_in0),

    J_REP_SPEC_MODE(as_range, constant),
    J_REP_SPEC_MODE(as_vec, constant),

    J_REP_SPEC_MODE(progn, progn),
    J_REP_SPEC_MODE(prog1, in0),
    J_REP_SPEC_MODE(call, constant),
    J_REP_SPEC_MODE(full_call, constant),

    J_REP_SPEC_MODE(vec, constant),
    J_REP_SPEC_MODE(vec_build, constant),
    J_REP_SPEC_MODE(make_vec, constant),
    J_REP_SPEC_MODE(range_copy, constant),
    J_REP_SPEC_MODE(vec_append, constant),

    J_REP_SPEC_MODE(range_length, constant),
    J_REP_SPEC_MODE(range_get, constant),
    J_REP_SPEC_MODE(range_set, in2),

    J_REP_SPEC_MODE(closure, constant),
    J_REP_SPEC_MODE(act_rec_alloc, constant),
    J_REP_SPEC_MODE(act_rec_rd, constant),
    J_REP_SPEC_MODE(act_rec_wr, type_in1),
    J_REP_SPEC_MODE(act_rec_parent, constant),
  };

#undef J_REP_SPEC_MODE
}
