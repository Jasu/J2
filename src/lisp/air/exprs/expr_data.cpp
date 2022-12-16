#include "lisp/air/exprs/expr_data.hpp"
#include "lisp/values/lisp_imms.hpp"

#define EXPR_

#define NUM(P) P"0", P"1", P"2", P"3", P"4", P"5"

#define EXPR_PARAMS_PROGN .inputs = {NUM("Form")}
#define EXPR_PARAMS_FNARGS .inputs ={NUM("Arg")}
#define EXPR_PARAMS_VEC .inputs = {NUM("Vec")}
#define EXPR_PARAMS_NUM .inputs = {NUM("Num")}
#define EXPR_FN_BODY_ARGS .inputs = {NUM("Arg")}

#define EXPR_FLAG_WR               expr_flag::writes_memory
#define EXPR_FLAG_RD               expr_flag::reads_memory
#define EXPR_FLAG_LEX_WR           expr_flag::writes_lex
#define EXPR_FLAG_LEX_RD           expr_flag::reads_lex
#define EXPR_FLAG_SIDE             expr_flag::has_side_effects
#define EXPR_FLAG_NLX              expr_flag::may_nlx
#define EXPR_FLAG_MOVABLE_NLX      expr_flag::movable_nlx
#define EXPR_FLAG_ASS              expr_flag::associative
#define EXPR_FLAG_COM              expr_flag::commutative
#define EXPR_FLAG_COM_TAIL         expr_flag::commutative_tail
#define EXPR_FLAG_IDEM             expr_flag::idempotent
#define EXPR_FLAG_COMP_IDEM        expr_flag::composition_idempotent
#define EXPR_FLAG_INV              expr_flag::involutory
#define EXPR_FLAG_PROGNISH         expr_flag::prognish
#define EXPR_FLAG_SPECIAL          expr_flag::is_special
#define EXPR_FLAG_HAS_VARS         expr_flag::has_vars
#define EXPR_FLAG_UNARY_ID         expr_flag::unary_identity
#define EXPR_FLAG_HAS_ID_VAL       expr_flag::has_identity_element
#define EXPR_FLAG_HAS_ID_VAL_TAIL  expr_flag::has_identity_element_tail
#define EXPR_FLAG_HAS_AB_VAL       expr_flag::has_absorbing_element
#define EXPR_FLAG_IS_UNARY_C       expr_flag::is_unary_constant
#define EXPR_FLAG_IS_NULLARY_C     expr_flag::is_nullary_constant
#define EXPR_FLAG(F) EXPR_FLAG_##F

#define EXPR_FOLD1 .fold = { 1U }
#define EXPR_FOLD2 .fold = { 2U }
#define EXPR_FOLD2_TAIL(TAIL) .fold = { 2U, expr_type::TAIL }
#define EXPR_FLAGS(...) .flags = { J_FEC_2(EXPR_FLAG, __VA_ARGS__) }
#define EXPR_PARAMS(...) .inputs = { __VA_ARGS__ }
#define EXPR_STATICS(...) .statics = { __VA_ARGS__ }
#define EXPR_UNARY_ID(R) .unary_result = R
#define EXPR_ID_ELEM(R) .identity_element = R
#define EXPR_ABS_ELEM(R) .absorbing_element = R
#define EXPR_NULL_ID(R) .nullary_result = R

#define EXPR_ZERO lisp_i64(0)
#define EXPR_ONE lisp_i64(1)
#define EXPR_TRUE lisp_bool(true)
#define EXPR_FALSE lisp_bool(false)
#define EXPR_NIL lisp_nil()

#define EXPR_APPLY(...) EXPR_##__VA_ARGS__
#define EXPR(T, ...) [(u8_t)expr_type::T] = {J_FEC(EXPR_APPLY, __VA_ARGS__)}

namespace j::lisp::air::exprs {
  static const inline lisp_i64 izero(0);
  static const inline lisp_i64 ione(1);
  static const inline lisp_bool btrue(true);
  static const inline lisp_bool bfalse(false);
  static const inline lisp_nil nil;
  const expr_data_table expr_data_v{{
    EXPR(nop),

    EXPR(fn_body, PARAMS_PROGN, STATICS("Args"), FLAGS(PROGNISH, HAS_VARS)),
    EXPR(fn_arg, STATICS("Arg#")),
    EXPR(fn_rest_arg),
    EXPR(fn_sc_arg),

    EXPR(let,    PARAMS_PROGN, STATICS("Lex"), FLAGS(PROGNISH, HAS_VARS)),
    EXPR(lex_rd, STATICS("Lex"), FLAGS(LEX_RD)),
    EXPR(lex_wr, PARAMS("Val"), STATICS("Lex"), FLAGS(LEX_WR)),
    EXPR(lex_rd_bound, STATICS("Lex"), FLAGS(RD)),
    EXPR(lex_wr_bound, PARAMS("Val"), STATICS("Lex"), FLAGS(WR)),

    EXPR(sym_val_rd, STATICS("Id"), FLAGS(RD)),
    EXPR(sym_val_wr, PARAMS("Val"), STATICS("Id"), FLAGS(WR)),

    EXPR(ld_const, STATICS("Const")),

    EXPR(ineg, PARAMS("Val"), FLAGS(INV), FOLD1),
    EXPR(iadd, PARAMS("Lhs", "Rhs"), FLAGS(COM, ASS, HAS_ID_VAL), ID_ELEM(izero), FOLD2),
    EXPR(isub, PARAMS("Lhs", "Rhs"), FLAGS(COM_TAIL, HAS_ID_VAL_TAIL), ID_ELEM(izero), FOLD2_TAIL(iadd)),
    EXPR(imul, PARAMS("Lhs", "Rhs"), FLAGS(COM, ASS, HAS_ID_VAL, HAS_AB_VAL), ID_ELEM(ione), ABS_ELEM(izero), FOLD2),
    EXPR(idiv, PARAMS("Lhs", "Rhs"), FLAGS(COM_TAIL), FOLD2_TAIL(imul)),

    EXPR(eq, PARAMS("Lhs", "Rhs"), FLAGS(COM, IDEM, IS_NULLARY_C, IS_UNARY_C), NULL_ID(btrue), UNARY_ID(btrue), FOLD2),
    EXPR(neq, PARAMS("Lhs", "Rhs"), FLAGS(COM, IS_NULLARY_C, IS_UNARY_C), NULL_ID(bfalse), UNARY_ID(bfalse), FOLD2),

    EXPR(tag, PARAMS("Val"), FLAGS(COMP_IDEM)),
    EXPR(untag, PARAMS("Val"), FLAGS(COMP_IDEM)),
    EXPR(as_range, PARAMS("Vec"), FLAGS(COMP_IDEM)),
    EXPR(as_vec, PARAMS("Range"), FLAGS(COMP_IDEM, MOVABLE_NLX, RD)),

    EXPR(progn, PARAMS_PROGN, FLAGS(ASS, COMP_IDEM, UNARY_ID, IS_NULLARY_C, PROGNISH), NULL_ID(nil)),
    EXPR(prog1, PARAMS_PROGN, FLAGS(UNARY_ID, IS_NULLARY_C), NULL_ID(nil)),
    EXPR(call, PARAMS_FNARGS, FLAGS(SIDE, RD, WR, NLX)),
    EXPR(full_call, PARAMS_FNARGS, STATICS("Fn"), FLAGS(SIDE, RD, WR, NLX)),

    // The only difference between type_assert and dyn_type_assert is NLX vs. MOVABLE_NLX.
    EXPR(type_assert, PARAMS("Val"), STATICS("Types"), FLAGS(COMP_IDEM, NLX)),
    EXPR(dyn_type_assert, PARAMS("Val"), STATICS("Types"), FLAGS(COMP_IDEM, MOVABLE_NLX)),

    EXPR(b_if, PARAMS("Cond", "Then", "Else"), FLAGS(SPECIAL)),
    EXPR(do_until, PARAMS("Loop"), FLAGS(SPECIAL)),

    EXPR(lor,  PARAMS_PROGN, FLAGS(COMP_IDEM, UNARY_ID, ASS, IS_NULLARY_C, SPECIAL), NULL_ID(bfalse), FOLD2),
    EXPR(land, PARAMS_PROGN, FLAGS(COMP_IDEM, UNARY_ID, ASS, IS_NULLARY_C, SPECIAL), NULL_ID(btrue), FOLD2),
    EXPR(lnot, PARAMS("Val"), FOLD1),

    EXPR(to_bool, PARAMS("Val"), FLAGS(COMP_IDEM), FOLD1),

    EXPR(vec,        PARAMS_PROGN, STATICS("Elems"), FLAGS(MOVABLE_NLX)),
    EXPR(vec_build,  PARAMS_PROGN, STATICS("Opers"), FLAGS(RD, MOVABLE_NLX)),
    EXPR(make_vec,   PARAMS("Sz"), FLAGS(MOVABLE_NLX)),
    EXPR(vec_append, PARAMS_VEC, FLAGS(ASS, COMP_IDEM, RD, MOVABLE_NLX)),

    EXPR(range_copy, PARAMS("Rng"), FLAGS(COMP_IDEM, RD, MOVABLE_NLX)),
    EXPR(range_length, PARAMS("Rng"), FOLD1),
    EXPR(range_get,  PARAMS("Rng", "Idx"), FLAGS(RD), FOLD2),
    EXPR(range_set,  PARAMS("Rng", "Idx", "Val"), FLAGS(WR)),

    EXPR(act_rec_alloc, PARAMS("Parent"), STATICS("Rec idx"), FLAGS(MOVABLE_NLX)),
    EXPR(act_rec_wr, PARAMS("Rec", "Value"), STATICS("Var"), FLAGS(WR)),
    EXPR(act_rec_rd, PARAMS("Rec"), STATICS("Var"), FLAGS(RD)),
    EXPR(act_rec_parent, PARAMS("Rec"), FLAGS(RD)),

    EXPR(closure,    PARAMS("Rec", "Body"), STATICS("Info"), FLAGS(RD)),

    EXPR(none),
  }};
}
