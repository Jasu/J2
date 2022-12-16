#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "lisp/air/exprs/expr_info.hpp"

namespace j::lisp::air::exprs {
  J_A(ND) constinit const expr_size_info expr_sizes_v[num_expr_types_v]{
    // nop()
    [(u8_t)expr_type::nop]             = {24U, false, 0, false},
    // ld_const(const_val)
    [(u8_t)expr_type::ld_const]        = {32U, false, 0, false},
    // fn_arg(index)
    [(u8_t)expr_type::fn_arg]          = {28U, false, 0, false},
    // fn_rest_arg()
    [(u8_t)expr_type::fn_rest_arg]     = {24U, false, 0, false},
    // fn_sc_arg()
    [(u8_t)expr_type::fn_sc_arg]       = {24U, false, 0, false},
    // fn_body(forms, vars)
    [(u8_t)expr_type::fn_body]         = {24U, true,  0, true },
    // progn(forms)
    [(u8_t)expr_type::progn]           = {24U, true,  0, false},
    // prog1(forms)
    [(u8_t)expr_type::prog1]           = {24U, true,  0, false},
    // let(forms, vars)
    [(u8_t)expr_type::let]             = {24U, true,  0, true },
    // b_if(cond, then_branch, else_branch)
    [(u8_t)expr_type::b_if]            = {72U, false, 3, false},
    // do_until(branch)
    [(u8_t)expr_type::do_until]        = {40U, false, 1, false},
    // lex_rd(var)
    [(u8_t)expr_type::lex_rd]          = {48U, false, 0, false},
    // lex_rd_bound(var)
    [(u8_t)expr_type::lex_rd_bound]    = {48U, false, 0, false},
    // lex_wr(var, value)
    [(u8_t)expr_type::lex_wr]          = {64U, false, 1, false},
    // lex_wr_bound(var, value)
    [(u8_t)expr_type::lex_wr_bound]    = {64U, false, 1, false},
    // sym_val_rd(var)
    [(u8_t)expr_type::sym_val_rd]      = {28U, false, 0, false},
    // sym_val_wr(var, value)
    [(u8_t)expr_type::sym_val_wr]      = {44U, false, 1, false},
    // type_assert(value, types)
    [(u8_t)expr_type::type_assert]     = {42U, false, 1, false},
    // dyn_type_assert(value, types)
    [(u8_t)expr_type::dyn_type_assert] = {42U, false, 1, false},
    // ineg(value)
    [(u8_t)expr_type::ineg]            = {40U, false, 1, false},
    // iadd(lhs, rhs)
    [(u8_t)expr_type::iadd]            = {56U, false, 2, false},
    // isub(lhs, rhs)
    [(u8_t)expr_type::isub]            = {56U, false, 2, false},
    // imul(lhs, rhs)
    [(u8_t)expr_type::imul]            = {56U, false, 2, false},
    // idiv(lhs, rhs)
    [(u8_t)expr_type::idiv]            = {56U, false, 2, false},
    // lor(values)
    [(u8_t)expr_type::lor]             = {24U, true,  0, false},
    // land(values)
    [(u8_t)expr_type::land]            = {24U, true,  0, false},
    // lnot(value)
    [(u8_t)expr_type::lnot]            = {40U, false, 1, false},
    // to_bool(value)
    [(u8_t)expr_type::to_bool]         = {40U, false, 1, false},
    // eq(lhs, rhs)
    [(u8_t)expr_type::eq]              = {56U, false, 2, false},
    // neq(lhs, rhs)
    [(u8_t)expr_type::neq]             = {56U, false, 2, false},
    // tag(untagged)
    [(u8_t)expr_type::tag]             = {40U, false, 1, false},
    // untag(tagged)
    [(u8_t)expr_type::untag]           = {40U, false, 1, false},
    // as_range(vec)
    [(u8_t)expr_type::as_range]        = {40U, false, 1, false},
    // as_vec(range)
    [(u8_t)expr_type::as_vec]          = {40U, false, 1, false},
    // call(closure, args)
    [(u8_t)expr_type::call]            = {40U, true,  1, false},
    // full_call(args, fn)
    [(u8_t)expr_type::full_call]       = {28U, true,  0, false},
    // vec(elems)
    [(u8_t)expr_type::vec]             = {24U, true,  0, false},
    // make_vec(size)
    [(u8_t)expr_type::make_vec]        = {40U, false, 1, false},
    // vec_append(vecs)
    [(u8_t)expr_type::vec_append]      = {24U, true,  0, false},
    // vec_build(elems, quasi_info)
    [(u8_t)expr_type::vec_build]       = {40U, true,  0, false},
    // range_length(range)
    [(u8_t)expr_type::range_length]    = {40U, false, 1, false},
    // range_get(range, index)
    [(u8_t)expr_type::range_get]       = {56U, false, 2, false},
    // range_set(range, index, value)
    [(u8_t)expr_type::range_set]       = {72U, false, 3, false},
    // range_copy(range)
    [(u8_t)expr_type::range_copy]      = {40U, false, 1, false},
    // act_rec_alloc(index, parent)
    [(u8_t)expr_type::act_rec_alloc]   = {44U, false, 1, false},
    // act_rec_wr(var, act_rec, value)
    [(u8_t)expr_type::act_rec_wr]      = {64U, false, 2, false},
    // act_rec_rd(var, act_rec)
    [(u8_t)expr_type::act_rec_rd]      = {48U, false, 1, false},
    // act_rec_parent(act_rec)
    [(u8_t)expr_type::act_rec_parent]  = {40U, false, 1, false},
    // closure(act_rec, body, closure)
    [(u8_t)expr_type::closure]         = {64U, false, 2, false},
  };
}
