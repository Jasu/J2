#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "lisp/air/exprs/expr_info.hpp"

namespace j::lisp::air::exprs {
  J_A(ND) constexpr inline expr_init_data expr_init_data_v[num_expr_types_v]{
    // nop()
    [(u8_t)expr_type::nop] = {{imm_type_mask(0x8), rep_spec(0x1)}},
    // ld_const(const_val)
    [(u8_t)expr_type::ld_const] = {{imm_type_mask(0x3FF), rep_spec(0x7)}},
    // fn_arg(index)
    [(u8_t)expr_type::fn_arg] = {{imm_type_mask(0x3FF), rep_spec(0x1)}},
    // fn_rest_arg()
    [(u8_t)expr_type::fn_rest_arg] = {{imm_type_mask(0x800), rep_spec(0x4)}},
    // fn_sc_arg()
    [(u8_t)expr_type::fn_sc_arg] = {{imm_type_mask(0x400), rep_spec(0x2)}},
    // fn_body(forms, vars)
    [(u8_t)expr_type::fn_body] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // progn(forms)
    [(u8_t)expr_type::progn] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // prog1(forms)
    [(u8_t)expr_type::prog1] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // let(forms, vars)
    [(u8_t)expr_type::let] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // b_if(cond, then_branch, else_branch)
    [(u8_t)expr_type::b_if] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // do_until(branch)
    [(u8_t)expr_type::do_until] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // lex_rd(var)
    [(u8_t)expr_type::lex_rd] = {{imm_type_mask(0xFFF), rep_spec(0x7)}},
    // lex_rd_bound(var)
    [(u8_t)expr_type::lex_rd_bound] = {{imm_type_mask(0xFFF), rep_spec(0x7)}},
    // lex_wr(var, value)
    [(u8_t)expr_type::lex_wr] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // lex_wr_bound(var, value)
    [(u8_t)expr_type::lex_wr_bound] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // sym_val_rd(var)
    [(u8_t)expr_type::sym_val_rd] = {{imm_type_mask(0x3FF), rep_spec(0x1)}},
    // sym_val_wr(var, value)
    [(u8_t)expr_type::sym_val_wr] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // type_assert(value, types)
    [(u8_t)expr_type::type_assert] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // dyn_type_assert(value, types)
    [(u8_t)expr_type::dyn_type_assert] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // ineg(value)
    [(u8_t)expr_type::ineg] = {{imm_type_mask(0x1), rep_spec(0x3)},
        {{imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}}},
    // iadd(lhs, rhs)
    [(u8_t)expr_type::iadd] = {{imm_type_mask(0x1), rep_spec(0x3)},
        {{imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}}},
    // isub(lhs, rhs)
    [(u8_t)expr_type::isub] = {{imm_type_mask(0x1), rep_spec(0x3)},
        {{imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}}},
    // imul(lhs, rhs)
    [(u8_t)expr_type::imul] = {{imm_type_mask(0x1), rep_spec(0x2)},
        {{imm_type_mask(0x1), rep_spec(0x2)}, {imm_type_mask(0x1), rep_spec(0x2)}, {imm_type_mask(0x1), rep_spec(0x2)}}},
    // idiv(lhs, rhs)
    [(u8_t)expr_type::idiv] = {{imm_type_mask(0x1), rep_spec(0x2)},
        {{imm_type_mask(0x1), rep_spec(0x2)}, {imm_type_mask(0x1), rep_spec(0x2)}, {imm_type_mask(0x1), rep_spec(0x2)}}},
    // lor(values)
    [(u8_t)expr_type::lor] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // land(values)
    [(u8_t)expr_type::land] = {{imm_type_mask(0xFFF), rep_spec(0x7)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // lnot(value)
    [(u8_t)expr_type::lnot] = {{imm_type_mask(0x4), rep_spec(0x1)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // to_bool(value)
    [(u8_t)expr_type::to_bool] = {{imm_type_mask(0x4), rep_spec(0x1)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // eq(lhs, rhs)
    [(u8_t)expr_type::eq] = {{imm_type_mask(0x4), rep_spec(0x1)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // neq(lhs, rhs)
    [(u8_t)expr_type::neq] = {{imm_type_mask(0x4), rep_spec(0x1)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // tag(untagged)
    [(u8_t)expr_type::tag] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x3FF), rep_spec(0x2)}, {imm_type_mask(0x3FF), rep_spec(0x2)}, {imm_type_mask(0x3FF), rep_spec(0x2)}}},
    // untag(tagged)
    [(u8_t)expr_type::untag] = {{imm_type_mask(0x3FF), rep_spec(0x2)},
        {{imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // as_range(vec)
    [(u8_t)expr_type::as_range] = {{imm_type_mask(0x100), rep_spec(0x4)},
        {{imm_type_mask(0x100), rep_spec(0x3)}, {imm_type_mask(0x100), rep_spec(0x3)}, {imm_type_mask(0x100), rep_spec(0x3)}}},
    // as_vec(range)
    [(u8_t)expr_type::as_vec] = {{imm_type_mask(0x100), rep_spec(0x3)},
        {{imm_type_mask(0x800), rep_spec(0x4)}, {imm_type_mask(0x800), rep_spec(0x4)}, {imm_type_mask(0x800), rep_spec(0x4)}}},
    // call(closure, args)
    [(u8_t)expr_type::call] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x40), rep_spec(0x2)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // full_call(args, fn)
    [(u8_t)expr_type::full_call] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // vec(elems)
    [(u8_t)expr_type::vec] = {{imm_type_mask(0x100), rep_spec(0x2)},
        {{imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // make_vec(size)
    [(u8_t)expr_type::make_vec] = {{imm_type_mask(0x100), rep_spec(0x6)},
        {{imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}}},
    // vec_append(vecs)
    [(u8_t)expr_type::vec_append] = {{imm_type_mask(0x100), rep_spec(0x2)},
        {{imm_type_mask(0x100), rep_spec(0x2)}, {imm_type_mask(0x100), rep_spec(0x2)}, {imm_type_mask(0x100), rep_spec(0x2)}}},
    // vec_build(elems, quasi_info)
    [(u8_t)expr_type::vec_build] = {{imm_type_mask(0x100), rep_spec(0x2)},
        {{imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}, {imm_type_mask(0xFFF), rep_spec(0x7)}}},
    // range_length(range)
    [(u8_t)expr_type::range_length] = {{imm_type_mask(0x1), rep_spec(0x2)},
        {{imm_type_mask(0x900), rep_spec(0x7)}, {imm_type_mask(0x900), rep_spec(0x7)}, {imm_type_mask(0x900), rep_spec(0x7)}}},
    // range_get(range, index)
    [(u8_t)expr_type::range_get] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x900), rep_spec(0x7)}, {imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x1), rep_spec(0x3)}}},
    // range_set(range, index, value)
    [(u8_t)expr_type::range_set] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x900), rep_spec(0x7)}, {imm_type_mask(0x1), rep_spec(0x3)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // range_copy(range)
    [(u8_t)expr_type::range_copy] = {{imm_type_mask(0x100), rep_spec(0x7)},
        {{imm_type_mask(0x900), rep_spec(0x6)}, {imm_type_mask(0x900), rep_spec(0x6)}, {imm_type_mask(0x900), rep_spec(0x6)}}},
    // act_rec_alloc(index, parent)
    [(u8_t)expr_type::act_rec_alloc] = {{imm_type_mask(0x400), rep_spec(0x2)},
        {{imm_type_mask(0x408), rep_spec(0x3)}, {imm_type_mask(0x408), rep_spec(0x3)}, {imm_type_mask(0x408), rep_spec(0x3)}}},
    // act_rec_wr(var, act_rec, value)
    [(u8_t)expr_type::act_rec_wr] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x400), rep_spec(0x2)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
    // act_rec_rd(var, act_rec)
    [(u8_t)expr_type::act_rec_rd] = {{imm_type_mask(0x3FF), rep_spec(0x1)},
        {{imm_type_mask(0x400), rep_spec(0x2)}, {imm_type_mask(0x400), rep_spec(0x2)}, {imm_type_mask(0x400), rep_spec(0x2)}}},
    // act_rec_parent(act_rec)
    [(u8_t)expr_type::act_rec_parent] = {{imm_type_mask(0x400), rep_spec(0x2)},
        {{imm_type_mask(0x400), rep_spec(0x2)}, {imm_type_mask(0x400), rep_spec(0x2)}, {imm_type_mask(0x400), rep_spec(0x2)}}},
    // closure(act_rec, body, closure)
    [(u8_t)expr_type::closure] = {{imm_type_mask(0x40), rep_spec(0x3)},
        {{imm_type_mask(0x408), rep_spec(0x3)}, {imm_type_mask(0x3FF), rep_spec(0x1)}, {imm_type_mask(0x3FF), rep_spec(0x1)}}},
  };
}
