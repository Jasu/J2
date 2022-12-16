#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "lisp/air/exprs/exprs.hpp"

namespace j::lisp::air::exprs {
  /// Visit `expr *` as its real type (defined in `exprs.hpp`).
  template<typename Fn, typename... Args>
  J_A(ND) inline auto visit(Fn && fn, expr * J_NOT_NULL e, Args... args) {
    switch (e->type) {
    case expr_type::nop:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::nop*>(e), args...);
    case expr_type::ld_const:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::ld_const*>(e), args...);
    case expr_type::fn_arg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::fn_arg*>(e), args...);
    case expr_type::fn_rest_arg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::fn_rest_arg*>(e), args...);
    case expr_type::fn_sc_arg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::fn_sc_arg*>(e), args...);
    case expr_type::fn_body:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::fn_body*>(e), args...);
    case expr_type::progn:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::progn*>(e), args...);
    case expr_type::prog1:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::prog1*>(e), args...);
    case expr_type::let:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::let*>(e), args...);
    case expr_type::b_if:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::b_if*>(e), args...);
    case expr_type::do_until:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::do_until*>(e), args...);
    case expr_type::lex_rd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::lex_rd*>(e), args...);
    case expr_type::lex_rd_bound:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::lex_rd_bound*>(e), args...);
    case expr_type::lex_wr:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::lex_wr*>(e), args...);
    case expr_type::lex_wr_bound:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::lex_wr_bound*>(e), args...);
    case expr_type::sym_val_rd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::sym_val_rd*>(e), args...);
    case expr_type::sym_val_wr:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::sym_val_wr*>(e), args...);
    case expr_type::type_assert:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::type_assert*>(e), args...);
    case expr_type::dyn_type_assert:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::dyn_type_assert*>(e), args...);
    case expr_type::ineg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::ineg*>(e), args...);
    case expr_type::iadd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::iadd*>(e), args...);
    case expr_type::isub:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::isub*>(e), args...);
    case expr_type::imul:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::imul*>(e), args...);
    case expr_type::idiv:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::idiv*>(e), args...);
    case expr_type::lor:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::lor*>(e), args...);
    case expr_type::land:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::land*>(e), args...);
    case expr_type::lnot:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::lnot*>(e), args...);
    case expr_type::to_bool:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::to_bool*>(e), args...);
    case expr_type::eq:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::eq*>(e), args...);
    case expr_type::neq:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::neq*>(e), args...);
    case expr_type::tag:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::tag*>(e), args...);
    case expr_type::untag:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::untag*>(e), args...);
    case expr_type::as_range:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::as_range*>(e), args...);
    case expr_type::as_vec:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::as_vec*>(e), args...);
    case expr_type::call:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::call*>(e), args...);
    case expr_type::full_call:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::full_call*>(e), args...);
    case expr_type::vec:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::vec*>(e), args...);
    case expr_type::make_vec:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::make_vec*>(e), args...);
    case expr_type::vec_append:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::vec_append*>(e), args...);
    case expr_type::vec_build:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::vec_build*>(e), args...);
    case expr_type::range_length:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::range_length*>(e), args...);
    case expr_type::range_get:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::range_get*>(e), args...);
    case expr_type::range_set:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::range_set*>(e), args...);
    case expr_type::range_copy:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::range_copy*>(e), args...);
    case expr_type::act_rec_alloc:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::act_rec_alloc*>(e), args...);
    case expr_type::act_rec_wr:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::act_rec_wr*>(e), args...);
    case expr_type::act_rec_rd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::act_rec_rd*>(e), args...);
    case expr_type::act_rec_parent:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::act_rec_parent*>(e), args...);
    case expr_type::closure:
      return static_cast<Fn &&>(fn)(reinterpret_cast<exprs::closure*>(e), args...);
    case expr_type::none: J_FAIL("expr_type::none");
    }
  }

  /// Visit `const expr *` as its real type (defined in `exprs.hpp`).
  template<typename Fn, typename... Args>
  J_A(ND) inline auto visit(Fn && fn, const expr * J_NOT_NULL e, Args... args) {
    switch (e->type) {
    case expr_type::nop:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::nop*>(e), args...);
    case expr_type::ld_const:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::ld_const*>(e), args...);
    case expr_type::fn_arg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::fn_arg*>(e), args...);
    case expr_type::fn_rest_arg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::fn_rest_arg*>(e), args...);
    case expr_type::fn_sc_arg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::fn_sc_arg*>(e), args...);
    case expr_type::fn_body:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::fn_body*>(e), args...);
    case expr_type::progn:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::progn*>(e), args...);
    case expr_type::prog1:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::prog1*>(e), args...);
    case expr_type::let:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::let*>(e), args...);
    case expr_type::b_if:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::b_if*>(e), args...);
    case expr_type::do_until:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::do_until*>(e), args...);
    case expr_type::lex_rd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::lex_rd*>(e), args...);
    case expr_type::lex_rd_bound:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::lex_rd_bound*>(e), args...);
    case expr_type::lex_wr:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::lex_wr*>(e), args...);
    case expr_type::lex_wr_bound:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::lex_wr_bound*>(e), args...);
    case expr_type::sym_val_rd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::sym_val_rd*>(e), args...);
    case expr_type::sym_val_wr:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::sym_val_wr*>(e), args...);
    case expr_type::type_assert:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::type_assert*>(e), args...);
    case expr_type::dyn_type_assert:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::dyn_type_assert*>(e), args...);
    case expr_type::ineg:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::ineg*>(e), args...);
    case expr_type::iadd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::iadd*>(e), args...);
    case expr_type::isub:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::isub*>(e), args...);
    case expr_type::imul:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::imul*>(e), args...);
    case expr_type::idiv:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::idiv*>(e), args...);
    case expr_type::lor:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::lor*>(e), args...);
    case expr_type::land:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::land*>(e), args...);
    case expr_type::lnot:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::lnot*>(e), args...);
    case expr_type::to_bool:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::to_bool*>(e), args...);
    case expr_type::eq:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::eq*>(e), args...);
    case expr_type::neq:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::neq*>(e), args...);
    case expr_type::tag:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::tag*>(e), args...);
    case expr_type::untag:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::untag*>(e), args...);
    case expr_type::as_range:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::as_range*>(e), args...);
    case expr_type::as_vec:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::as_vec*>(e), args...);
    case expr_type::call:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::call*>(e), args...);
    case expr_type::full_call:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::full_call*>(e), args...);
    case expr_type::vec:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::vec*>(e), args...);
    case expr_type::make_vec:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::make_vec*>(e), args...);
    case expr_type::vec_append:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::vec_append*>(e), args...);
    case expr_type::vec_build:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::vec_build*>(e), args...);
    case expr_type::range_length:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::range_length*>(e), args...);
    case expr_type::range_get:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::range_get*>(e), args...);
    case expr_type::range_set:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::range_set*>(e), args...);
    case expr_type::range_copy:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::range_copy*>(e), args...);
    case expr_type::act_rec_alloc:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::act_rec_alloc*>(e), args...);
    case expr_type::act_rec_wr:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::act_rec_wr*>(e), args...);
    case expr_type::act_rec_rd:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::act_rec_rd*>(e), args...);
    case expr_type::act_rec_parent:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::act_rec_parent*>(e), args...);
    case expr_type::closure:
      return static_cast<Fn &&>(fn)(reinterpret_cast<const exprs::closure*>(e), args...);
    case expr_type::none: J_FAIL("expr_type::none");
    }
  }

  /// Get `expr*` as `nop*`, asserting on type mismatch.
  J_A(NODISC,RNN) nop * expr_as_nop(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const nop*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const nop * expr_as_nop(const expr * J_NOT_NULL e) noexcept
  { return expr_as_nop(const_cast<expr*>(e)); }

  /// Get `expr*` as `ld_const*`, asserting on type mismatch.
  J_A(NODISC,RNN) ld_const * expr_as_ld_const(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const ld_const*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const ld_const * expr_as_ld_const(const expr * J_NOT_NULL e) noexcept
  { return expr_as_ld_const(const_cast<expr*>(e)); }

  /// Get `expr*` as `fn_arg*`, asserting on type mismatch.
  J_A(NODISC,RNN) fn_arg * expr_as_fn_arg(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const fn_arg*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const fn_arg * expr_as_fn_arg(const expr * J_NOT_NULL e) noexcept
  { return expr_as_fn_arg(const_cast<expr*>(e)); }

  /// Get `expr*` as `fn_rest_arg*`, asserting on type mismatch.
  J_A(NODISC,RNN) fn_rest_arg * expr_as_fn_rest_arg(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const fn_rest_arg*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const fn_rest_arg * expr_as_fn_rest_arg(const expr * J_NOT_NULL e) noexcept
  { return expr_as_fn_rest_arg(const_cast<expr*>(e)); }

  /// Get `expr*` as `fn_sc_arg*`, asserting on type mismatch.
  J_A(NODISC,RNN) fn_sc_arg * expr_as_fn_sc_arg(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const fn_sc_arg*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const fn_sc_arg * expr_as_fn_sc_arg(const expr * J_NOT_NULL e) noexcept
  { return expr_as_fn_sc_arg(const_cast<expr*>(e)); }

  /// Get `expr*` as `fn_body*`, asserting on type mismatch.
  J_A(NODISC,RNN) fn_body * expr_as_fn_body(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const fn_body*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const fn_body * expr_as_fn_body(const expr * J_NOT_NULL e) noexcept
  { return expr_as_fn_body(const_cast<expr*>(e)); }

  /// Get `expr*` as `progn*`, asserting on type mismatch.
  J_A(NODISC,RNN) progn * expr_as_progn(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const progn*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const progn * expr_as_progn(const expr * J_NOT_NULL e) noexcept
  { return expr_as_progn(const_cast<expr*>(e)); }

  /// Get `expr*` as `prog1*`, asserting on type mismatch.
  J_A(NODISC,RNN) prog1 * expr_as_prog1(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const prog1*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const prog1 * expr_as_prog1(const expr * J_NOT_NULL e) noexcept
  { return expr_as_prog1(const_cast<expr*>(e)); }

  /// Get `expr*` as `let*`, asserting on type mismatch.
  J_A(NODISC,RNN) let * expr_as_let(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const let*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const let * expr_as_let(const expr * J_NOT_NULL e) noexcept
  { return expr_as_let(const_cast<expr*>(e)); }

  /// Get `expr*` as `b_if*`, asserting on type mismatch.
  J_A(NODISC,RNN) b_if * expr_as_b_if(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const b_if*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const b_if * expr_as_b_if(const expr * J_NOT_NULL e) noexcept
  { return expr_as_b_if(const_cast<expr*>(e)); }

  /// Get `expr*` as `do_until*`, asserting on type mismatch.
  J_A(NODISC,RNN) do_until * expr_as_do_until(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const do_until*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const do_until * expr_as_do_until(const expr * J_NOT_NULL e) noexcept
  { return expr_as_do_until(const_cast<expr*>(e)); }

  /// Get `expr*` as `lex_rd*`, asserting on type mismatch.
  J_A(NODISC,RNN) lex_rd * expr_as_lex_rd(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const lex_rd*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const lex_rd * expr_as_lex_rd(const expr * J_NOT_NULL e) noexcept
  { return expr_as_lex_rd(const_cast<expr*>(e)); }

  /// Get `expr*` as `lex_rd_bound*`, asserting on type mismatch.
  J_A(NODISC,RNN) lex_rd_bound * expr_as_lex_rd_bound(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const lex_rd_bound*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const lex_rd_bound * expr_as_lex_rd_bound(const expr * J_NOT_NULL e) noexcept
  { return expr_as_lex_rd_bound(const_cast<expr*>(e)); }

  /// Get `expr*` as `lex_wr*`, asserting on type mismatch.
  J_A(NODISC,RNN) lex_wr * expr_as_lex_wr(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const lex_wr*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const lex_wr * expr_as_lex_wr(const expr * J_NOT_NULL e) noexcept
  { return expr_as_lex_wr(const_cast<expr*>(e)); }

  /// Get `expr*` as `lex_wr_bound*`, asserting on type mismatch.
  J_A(NODISC,RNN) lex_wr_bound * expr_as_lex_wr_bound(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const lex_wr_bound*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const lex_wr_bound * expr_as_lex_wr_bound(const expr * J_NOT_NULL e) noexcept
  { return expr_as_lex_wr_bound(const_cast<expr*>(e)); }

  /// Get `expr*` as `sym_val_rd*`, asserting on type mismatch.
  J_A(NODISC,RNN) sym_val_rd * expr_as_sym_val_rd(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const sym_val_rd*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const sym_val_rd * expr_as_sym_val_rd(const expr * J_NOT_NULL e) noexcept
  { return expr_as_sym_val_rd(const_cast<expr*>(e)); }

  /// Get `expr*` as `sym_val_wr*`, asserting on type mismatch.
  J_A(NODISC,RNN) sym_val_wr * expr_as_sym_val_wr(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const sym_val_wr*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const sym_val_wr * expr_as_sym_val_wr(const expr * J_NOT_NULL e) noexcept
  { return expr_as_sym_val_wr(const_cast<expr*>(e)); }

  /// Get `expr*` as `type_assert*`, asserting on type mismatch.
  J_A(NODISC,RNN) type_assert * expr_as_type_assert(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const type_assert*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const type_assert * expr_as_type_assert(const expr * J_NOT_NULL e) noexcept
  { return expr_as_type_assert(const_cast<expr*>(e)); }

  /// Get `expr*` as `dyn_type_assert*`, asserting on type mismatch.
  J_A(NODISC,RNN) dyn_type_assert * expr_as_dyn_type_assert(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const dyn_type_assert*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const dyn_type_assert * expr_as_dyn_type_assert(const expr * J_NOT_NULL e) noexcept
  { return expr_as_dyn_type_assert(const_cast<expr*>(e)); }

  /// Get `expr*` as `ineg*`, asserting on type mismatch.
  J_A(NODISC,RNN) ineg * expr_as_ineg(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const ineg*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const ineg * expr_as_ineg(const expr * J_NOT_NULL e) noexcept
  { return expr_as_ineg(const_cast<expr*>(e)); }

  /// Get `expr*` as `iadd*`, asserting on type mismatch.
  J_A(NODISC,RNN) iadd * expr_as_iadd(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const iadd*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const iadd * expr_as_iadd(const expr * J_NOT_NULL e) noexcept
  { return expr_as_iadd(const_cast<expr*>(e)); }

  /// Get `expr*` as `isub*`, asserting on type mismatch.
  J_A(NODISC,RNN) isub * expr_as_isub(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const isub*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const isub * expr_as_isub(const expr * J_NOT_NULL e) noexcept
  { return expr_as_isub(const_cast<expr*>(e)); }

  /// Get `expr*` as `imul*`, asserting on type mismatch.
  J_A(NODISC,RNN) imul * expr_as_imul(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const imul*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const imul * expr_as_imul(const expr * J_NOT_NULL e) noexcept
  { return expr_as_imul(const_cast<expr*>(e)); }

  /// Get `expr*` as `idiv*`, asserting on type mismatch.
  J_A(NODISC,RNN) idiv * expr_as_idiv(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const idiv*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const idiv * expr_as_idiv(const expr * J_NOT_NULL e) noexcept
  { return expr_as_idiv(const_cast<expr*>(e)); }

  /// Get `expr*` as `lor*`, asserting on type mismatch.
  J_A(NODISC,RNN) lor * expr_as_lor(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const lor*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const lor * expr_as_lor(const expr * J_NOT_NULL e) noexcept
  { return expr_as_lor(const_cast<expr*>(e)); }

  /// Get `expr*` as `land*`, asserting on type mismatch.
  J_A(NODISC,RNN) land * expr_as_land(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const land*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const land * expr_as_land(const expr * J_NOT_NULL e) noexcept
  { return expr_as_land(const_cast<expr*>(e)); }

  /// Get `expr*` as `lnot*`, asserting on type mismatch.
  J_A(NODISC,RNN) lnot * expr_as_lnot(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const lnot*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const lnot * expr_as_lnot(const expr * J_NOT_NULL e) noexcept
  { return expr_as_lnot(const_cast<expr*>(e)); }

  /// Get `expr*` as `to_bool*`, asserting on type mismatch.
  J_A(NODISC,RNN) to_bool * expr_as_to_bool(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const to_bool*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const to_bool * expr_as_to_bool(const expr * J_NOT_NULL e) noexcept
  { return expr_as_to_bool(const_cast<expr*>(e)); }

  /// Get `expr*` as `eq*`, asserting on type mismatch.
  J_A(NODISC,RNN) eq * expr_as_eq(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const eq*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const eq * expr_as_eq(const expr * J_NOT_NULL e) noexcept
  { return expr_as_eq(const_cast<expr*>(e)); }

  /// Get `expr*` as `neq*`, asserting on type mismatch.
  J_A(NODISC,RNN) neq * expr_as_neq(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const neq*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const neq * expr_as_neq(const expr * J_NOT_NULL e) noexcept
  { return expr_as_neq(const_cast<expr*>(e)); }

  /// Get `expr*` as `tag*`, asserting on type mismatch.
  J_A(NODISC,RNN) tag * expr_as_tag(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const tag*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const tag * expr_as_tag(const expr * J_NOT_NULL e) noexcept
  { return expr_as_tag(const_cast<expr*>(e)); }

  /// Get `expr*` as `untag*`, asserting on type mismatch.
  J_A(NODISC,RNN) untag * expr_as_untag(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const untag*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const untag * expr_as_untag(const expr * J_NOT_NULL e) noexcept
  { return expr_as_untag(const_cast<expr*>(e)); }

  /// Get `expr*` as `as_range*`, asserting on type mismatch.
  J_A(NODISC,RNN) as_range * expr_as_as_range(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const as_range*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const as_range * expr_as_as_range(const expr * J_NOT_NULL e) noexcept
  { return expr_as_as_range(const_cast<expr*>(e)); }

  /// Get `expr*` as `as_vec*`, asserting on type mismatch.
  J_A(NODISC,RNN) as_vec * expr_as_as_vec(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const as_vec*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const as_vec * expr_as_as_vec(const expr * J_NOT_NULL e) noexcept
  { return expr_as_as_vec(const_cast<expr*>(e)); }

  /// Get `expr*` as `call*`, asserting on type mismatch.
  J_A(NODISC,RNN) call * expr_as_call(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const call*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const call * expr_as_call(const expr * J_NOT_NULL e) noexcept
  { return expr_as_call(const_cast<expr*>(e)); }

  /// Get `expr*` as `full_call*`, asserting on type mismatch.
  J_A(NODISC,RNN) full_call * expr_as_full_call(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const full_call*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const full_call * expr_as_full_call(const expr * J_NOT_NULL e) noexcept
  { return expr_as_full_call(const_cast<expr*>(e)); }

  /// Get `expr*` as `vec*`, asserting on type mismatch.
  J_A(NODISC,RNN) vec * expr_as_vec(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const vec*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const vec * expr_as_vec(const expr * J_NOT_NULL e) noexcept
  { return expr_as_vec(const_cast<expr*>(e)); }

  /// Get `expr*` as `make_vec*`, asserting on type mismatch.
  J_A(NODISC,RNN) make_vec * expr_as_make_vec(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const make_vec*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const make_vec * expr_as_make_vec(const expr * J_NOT_NULL e) noexcept
  { return expr_as_make_vec(const_cast<expr*>(e)); }

  /// Get `expr*` as `vec_append*`, asserting on type mismatch.
  J_A(NODISC,RNN) vec_append * expr_as_vec_append(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const vec_append*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const vec_append * expr_as_vec_append(const expr * J_NOT_NULL e) noexcept
  { return expr_as_vec_append(const_cast<expr*>(e)); }

  /// Get `expr*` as `vec_build*`, asserting on type mismatch.
  J_A(NODISC,RNN) vec_build * expr_as_vec_build(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const vec_build*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const vec_build * expr_as_vec_build(const expr * J_NOT_NULL e) noexcept
  { return expr_as_vec_build(const_cast<expr*>(e)); }

  /// Get `expr*` as `range_length*`, asserting on type mismatch.
  J_A(NODISC,RNN) range_length * expr_as_range_length(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const range_length*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const range_length * expr_as_range_length(const expr * J_NOT_NULL e) noexcept
  { return expr_as_range_length(const_cast<expr*>(e)); }

  /// Get `expr*` as `range_get*`, asserting on type mismatch.
  J_A(NODISC,RNN) range_get * expr_as_range_get(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const range_get*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const range_get * expr_as_range_get(const expr * J_NOT_NULL e) noexcept
  { return expr_as_range_get(const_cast<expr*>(e)); }

  /// Get `expr*` as `range_set*`, asserting on type mismatch.
  J_A(NODISC,RNN) range_set * expr_as_range_set(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const range_set*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const range_set * expr_as_range_set(const expr * J_NOT_NULL e) noexcept
  { return expr_as_range_set(const_cast<expr*>(e)); }

  /// Get `expr*` as `range_copy*`, asserting on type mismatch.
  J_A(NODISC,RNN) range_copy * expr_as_range_copy(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const range_copy*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const range_copy * expr_as_range_copy(const expr * J_NOT_NULL e) noexcept
  { return expr_as_range_copy(const_cast<expr*>(e)); }

  /// Get `expr*` as `act_rec_alloc*`, asserting on type mismatch.
  J_A(NODISC,RNN) act_rec_alloc * expr_as_act_rec_alloc(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const act_rec_alloc*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const act_rec_alloc * expr_as_act_rec_alloc(const expr * J_NOT_NULL e) noexcept
  { return expr_as_act_rec_alloc(const_cast<expr*>(e)); }

  /// Get `expr*` as `act_rec_wr*`, asserting on type mismatch.
  J_A(NODISC,RNN) act_rec_wr * expr_as_act_rec_wr(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const act_rec_wr*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const act_rec_wr * expr_as_act_rec_wr(const expr * J_NOT_NULL e) noexcept
  { return expr_as_act_rec_wr(const_cast<expr*>(e)); }

  /// Get `expr*` as `act_rec_rd*`, asserting on type mismatch.
  J_A(NODISC,RNN) act_rec_rd * expr_as_act_rec_rd(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const act_rec_rd*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const act_rec_rd * expr_as_act_rec_rd(const expr * J_NOT_NULL e) noexcept
  { return expr_as_act_rec_rd(const_cast<expr*>(e)); }

  /// Get `expr*` as `act_rec_parent*`, asserting on type mismatch.
  J_A(NODISC,RNN) act_rec_parent * expr_as_act_rec_parent(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const act_rec_parent*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const act_rec_parent * expr_as_act_rec_parent(const expr * J_NOT_NULL e) noexcept
  { return expr_as_act_rec_parent(const_cast<expr*>(e)); }

  /// Get `expr*` as `closure*`, asserting on type mismatch.
  J_A(NODISC,RNN) closure * expr_as_closure(expr * J_NOT_NULL e) noexcept;
  /// Get `const expr*` as `const closure*`, asserting on type mismatch.
  J_A(NODISC,RNN,AI,ND,HIDDEN) inline const closure * expr_as_closure(const expr * J_NOT_NULL e) noexcept
  { return expr_as_closure(const_cast<expr*>(e)); }

}
