#include "lisp/cir/ops/op_data.hpp"

#define OP(T, R, ...) [(u8_t)op_type::T] = {RES_##R, __VA_ARGS__}

#define ALIAS_NONE 0
#define ALIAS_ANY 0x5555U
#define ALIAS_ARGS 0x5554U
#define ALIAS_PARGS 0xFFFCU
#define ALIAS_A0 0x0004U
#define ALIAS_A1 0x0010U
#define ALIAS_A2 0x0040U
#define ALIAS_A3 0x0100U
#define ALIAS_P0 0x000CU
#define ALIAS_P1 0x0030U
#define ALIAS_P2 0x00C0U
#define ALIAS_P3 0x0300U
#define ALIAS_R 0x1U
#define ALIAS_PR 0x3U
#define ALIAS(A) ALIAS_##A
#define RES_NONE result_data{loc_specifier::none, type_mode::none, alias_flags(0U)}
#define RES_GPR(...) result_data{loc_specifier::any, type_mode::gpr, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define RES_ANY(...) result_data{loc_specifier::any, type_mode::any, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define RES_FP(...) result_data{loc_specifier::any, type_mode::fp, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}

#define PARAM_ANY(NAME, ...) param_data{NAME, loc_specifier::any, false, type_mode::any, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define PARAM_GPR(NAME, ...) param_data{NAME, loc_specifier::any, false, type_mode::gpr, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define PARAM_FP(NAME, ...) param_data{NAME, loc_specifier::any, false, type_mode::fp, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}

#define PARAM_ANY_C(NAME, ...) param_data{NAME, loc_specifier::any, true, type_mode::any, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define PARAM_ANY_C_PR(NAME) PARAM_ANY_C(NAME, ANY, PR)
#define PARAM_ANY_C_ANY(NAME) PARAM_ANY_C(NAME, ANY)
#define PARAM_GPR_C(NAME, ...) param_data{NAME, loc_specifier::any, true, type_mode::gpr, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define PARAM_GPR_C_ANY(NAME) PARAM_GPR_C(NAME, ANY, PR)
#define PARAM_FP_C(NAME, ...) param_data{NAME, loc_specifier::any, true, type_mode::fp, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}

#define AUX_ANY(NAME, ...) aux_reg_data{NAME, loc_specifier::any, type_mode::any, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define AUX_GPR(NAME, ...) aux_reg_data{NAME, loc_specifier::any, type_mode::gpr, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}
#define AUX_FP(NAME, ...) aux_reg_data{NAME, loc_specifier::any, type_mode::fp, alias_flags(J_FEC_2(ALIAS, __VA_ARGS__))}

#define LHS_ANY(...) PARAM_ANY("lhs", __VA_ARGS__)
#define LHS_FP(...) PARAM_FP("lhs", __VA_ARGS__)
#define LHS_GPR(...) PARAM_GPR("lhs", __VA_ARGS__)
#define RHS_ANY(...) PARAM_ANY("rhs", __VA_ARGS__)
#define RHS_FP(...) PARAM_FP("rhs", __VA_ARGS__)
#define RHS_GPR(...) PARAM_GPR("rhs", __VA_ARGS__)

#define LHS_ANY_C(...) PARAM_ANY_C("lhs", __VA_ARGS__)
#define LHS_FP_C(...) PARAM_FP_C("lhs", __VA_ARGS__)
#define LHS_GPR_C(...) PARAM_GPR_C("lhs", __VA_ARGS__)
#define RHS_ANY_C(...) PARAM_ANY_C("rhs", __VA_ARGS__)
#define RHS_FP_C(...) PARAM_FP_C("rhs", __VA_ARGS__)
#define RHS_GPR_C(...) PARAM_GPR_C("rhs", __VA_ARGS__)

#define DUMMY(...)
// #define PARAMS_IMPL(P1, P2, P3, P4, P5, P6, P7, P8, ...) P1, P2, P3, P4, P5, P6, P7, P8
// #define PARAMS_NUM_IMPL(N, P0, P1, P2, P3, P4, P5, P6, P7, ...)       \
//  P0(N"0"), P1(N"1"), P2(N"2"), P3(N"3"), P4(N"4"), P5(N"5"), P6(N"6"), P7(N"7")
#define PARAMS_IMPL(P1, P2, P3, P4, P5, P6, P7, ...) P1, P2, P3, P4, P5, P6, P7
#define PARAMS_NUM_IMPL(N, P0, P1, P2, P3, P4, P5, P6, ...) P0(N"0"), P1(N"1"), P2(N"2"), P3(N"3"), P4(N"4"), P5(N"5"), P6(N"6")
#define PARAMS(P, ...) PARAMS_IMPL(P, __VA_OPT__(,) __VA_ARGS__ P, P, P, P, P, P, P)
#define PARAMS_NUM(N, P, ...) PARAMS_NUM_IMPL(N, P, __VA_OPT__(,) __VA_ARGS__ P, P, P, P, P, P, P)
#define PARAMS_NUM_1(N, P, ...) PARAMS_NUM_IMPL(N, P, __VA_OPT__(,) __VA_ARGS__ P, P, P, P, P, DUMMY)

namespace j::lisp::cir::inline ops {
  constinit const op_data_table default_op_data{{
    OP(phi,         ANY(P0, P1),      { PARAMS_NUM("in",       PARAM_ANY_C_PR) }),

    OP(mov,         NONE,     {},                      { "dst",      "src" }),
    OP(push,        NONE,     {},                      { "type",     "reg" }),
    OP(pop,         NONE,     {},                      { "type",     "reg" }),
    OP(swap,        NONE,     {},                      { "reg0",     "reg1" }),

    OP(iconst,      GPR(ANY),      {},                      { "const" }),
    OP(fconst,      FP(ANY),       {},                      { "const" }),
    OP(mconst,      GPR(ANY),      {},                      { "const" }),

    OP(icopy,       GPR(P0),   { PARAM_GPR("value", PR) }),
    OP(fcopy,       FP(P0),       { PARAM_FP("value", PR) }),

    OP(fn_iarg,     GPR(ANY),      {},                      { "Index", "Has rest", "Has chain" }),
    OP(fn_farg,     FP(ANY),       {},                      { "Index" }),
    OP(fn_rest_ptr, GPR(ANY)),
    OP(fn_rest_len, GPR(ANY)),
    OP(fn_sc_ptr,   GPR(ANY),      {},                      { "Has rest" }),

    OP(alloca,      GPR(ANY),      { PARAMS_NUM("arg",             PARAM_ANY_C_ANY) }),
    OP(dealloca,    NONE,          { PARAM_GPR_C("arity", ANY) }),
    OP(call,        GPR(ANY),      { PARAM_GPR("fn", ANY, PR),       PARAMS_NUM_1("arg", PARAM_GPR_C_ANY) }),
    OP(full_call,   GPR(ANY),      { PARAM_GPR("fn", ANY, PR),       PARAM_GPR_C("nargs", ANY, PR),   PARAM_GPR_C("args", ANY) }),
    OP(abi_call,    GPR(ANY),      { PARAM_GPR("fn", ANY, PR),       PARAMS_NUM_1("arg", PARAM_GPR_C_ANY) }),

    OP(ineg,        GPR(P0),       {PARAM_GPR_C("val", PR)}),
    OP(iadd,        GPR(P0, P1),   {LHS_GPR_C(PR, A1),               RHS_GPR_C(PR, A0)}),
    OP(isub,        GPR(P0),       {LHS_GPR(A1, PR),                 RHS_GPR(A0)}),
    OP(smul,        GPR(P0, P1),   {LHS_GPR_C(A1, PR),               RHS_GPR_C(PR, A0)}),
    OP(sdiv,        GPR(P0),       {LHS_GPR_C(PR),                   RHS_GPR_C(NONE)}),
    OP(umul,        GPR(P0, P1),   {LHS_GPR_C(A1, PR),               RHS_GPR_C(PR, A0)}),
    OP(udiv,        GPR(P0),       {LHS_GPR_C(PR),                   RHS_GPR_C(NONE)}),

    OP(fadd,        FP(PARGS),       {LHS_FP_C(ARGS, PR),               RHS_FP_C(ARGS, PR)}),
    OP(fsub,        FP(P0),  {LHS_FP_C(ARGS, PR),            RHS_FP_C(ARGS)}),

    OP(shl,         GPR(P0), {LHS_GPR_C(PR),           RHS_GPR_C(NONE)}),
    OP(shr,         GPR(P0), {LHS_GPR_C(PR),           RHS_GPR_C(NONE)}),
    OP(sar,         GPR(P0), {LHS_GPR_C(PR),           RHS_GPR_C(NONE)}),

    OP(band,        GPR(P0, P1),      {LHS_GPR_C(PR, ARGS),              RHS_GPR_C(PR, ARGS)}),
    OP(bor,         GPR(P0, P1),      {LHS_GPR_C(PR, ARGS),              RHS_GPR_C(PR, ARGS)}),
    OP(bxor,        GPR(P0, P1),      {LHS_GPR_C(PR, ARGS),              RHS_GPR_C(PR, ARGS)}),
    OP(bnot,        GPR(P0),   {PARAM_GPR_C("value", PR)}),

    OP(eq,          GPR(PARGS),      {LHS_GPR_C(PR, ARGS),              RHS_GPR_C(PR, ARGS)}),
    OP(neq,         GPR(PARGS),      {LHS_GPR_C(PR, ARGS),              RHS_GPR_C(PR, ARGS)}),

    OP(tag,         GPR(P0),   {PARAM_GPR("raw", PR)},      { "type" }),
    OP(untag,       GPR(P0),   {PARAM_GPR("tagged", PR)},   { "type" }),

    OP(mem_ird,     GPR(P0),  {PARAM_GPR("ptr", PR)}),
    OP(mem_iwr,     NONE,     {PARAM_GPR("ptr", ARGS),       PARAM_GPR_C("value", ARGS)}),
    OP(mem_frd,     FP(P0),   {PARAM_GPR("ptr", PR)},      {"type"}),
    OP(mem_fwr,     NONE,     {PARAM_GPR("ptr", ARGS),       PARAM_FP("value", ARGS)},   {"type"}),

    OP(mem_copy64, GPR(P0), {PARAM_GPR("to", PR),       PARAM_GPR("from", NONE), PARAM_GPR_C("count", NONE)}),

    OP(lea,         GPR(P0), {PARAM_GPR("addr", PR)}),

    OP(debug_trap,  NONE),

    OP(type_error,  NONE,     {}, { "type" }),
    OP(iret,        NONE,     {PARAM_GPR("result", NONE)}),
    OP(fret,        NONE,     {PARAM_FP("result", NONE)}),

    OP(jmpc,        NONE,     {PARAM_GPR_C("lhs", ARGS),       PARAM_GPR_C("rhs", ARGS)},    { "cond" }),
    OP(jmpt,        NONE,     {PARAM_GPR_C("value", NONE)}),
    OP(jmptype,     NONE,     {PARAM_GPR_C("value", NONE)}, {"types"}, {AUX_GPR("temp", P0)}),
  }};
}
