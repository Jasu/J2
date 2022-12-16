// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/air/exprs/visit.hpp"
#include "exceptions/assert.hpp"

namespace j::lisp::air::exprs {
  J_A(NI) static void chk(expr * J_NOT_NULL e, expr_type type) noexcept {
    J_REQUIRE(e->type == type, "Invalid expression, expected {}, got {}.", type, e->type);
  }

  J_A(NODISC,RNN) nop * expr_as_nop(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::nop);
    return reinterpret_cast<nop*>(e);
  }

  J_A(NODISC,RNN) ld_const * expr_as_ld_const(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::ld_const);
    return reinterpret_cast<ld_const*>(e);
  }

  J_A(NODISC,RNN) fn_arg * expr_as_fn_arg(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::fn_arg);
    return reinterpret_cast<fn_arg*>(e);
  }

  J_A(NODISC,RNN) fn_rest_arg * expr_as_fn_rest_arg(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::fn_rest_arg);
    return reinterpret_cast<fn_rest_arg*>(e);
  }

  J_A(NODISC,RNN) fn_sc_arg * expr_as_fn_sc_arg(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::fn_sc_arg);
    return reinterpret_cast<fn_sc_arg*>(e);
  }

  J_A(NODISC,RNN) fn_body * expr_as_fn_body(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::fn_body);
    return reinterpret_cast<fn_body*>(e);
  }

  J_A(NODISC,RNN) progn * expr_as_progn(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::progn);
    return reinterpret_cast<progn*>(e);
  }

  J_A(NODISC,RNN) prog1 * expr_as_prog1(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::prog1);
    return reinterpret_cast<prog1*>(e);
  }

  J_A(NODISC,RNN) let * expr_as_let(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::let);
    return reinterpret_cast<let*>(e);
  }

  J_A(NODISC,RNN) b_if * expr_as_b_if(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::b_if);
    return reinterpret_cast<b_if*>(e);
  }

  J_A(NODISC,RNN) do_until * expr_as_do_until(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::do_until);
    return reinterpret_cast<do_until*>(e);
  }

  J_A(NODISC,RNN) lex_rd * expr_as_lex_rd(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::lex_rd);
    return reinterpret_cast<lex_rd*>(e);
  }

  J_A(NODISC,RNN) lex_rd_bound * expr_as_lex_rd_bound(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::lex_rd_bound);
    return reinterpret_cast<lex_rd_bound*>(e);
  }

  J_A(NODISC,RNN) lex_wr * expr_as_lex_wr(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::lex_wr);
    return reinterpret_cast<lex_wr*>(e);
  }

  J_A(NODISC,RNN) lex_wr_bound * expr_as_lex_wr_bound(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::lex_wr_bound);
    return reinterpret_cast<lex_wr_bound*>(e);
  }

  J_A(NODISC,RNN) sym_val_rd * expr_as_sym_val_rd(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::sym_val_rd);
    return reinterpret_cast<sym_val_rd*>(e);
  }

  J_A(NODISC,RNN) sym_val_wr * expr_as_sym_val_wr(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::sym_val_wr);
    return reinterpret_cast<sym_val_wr*>(e);
  }

  J_A(NODISC,RNN) type_assert * expr_as_type_assert(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::type_assert);
    return reinterpret_cast<type_assert*>(e);
  }

  J_A(NODISC,RNN) dyn_type_assert * expr_as_dyn_type_assert(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::dyn_type_assert);
    return reinterpret_cast<dyn_type_assert*>(e);
  }

  J_A(NODISC,RNN) ineg * expr_as_ineg(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::ineg);
    return reinterpret_cast<ineg*>(e);
  }

  J_A(NODISC,RNN) iadd * expr_as_iadd(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::iadd);
    return reinterpret_cast<iadd*>(e);
  }

  J_A(NODISC,RNN) isub * expr_as_isub(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::isub);
    return reinterpret_cast<isub*>(e);
  }

  J_A(NODISC,RNN) imul * expr_as_imul(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::imul);
    return reinterpret_cast<imul*>(e);
  }

  J_A(NODISC,RNN) idiv * expr_as_idiv(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::idiv);
    return reinterpret_cast<idiv*>(e);
  }

  J_A(NODISC,RNN) lor * expr_as_lor(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::lor);
    return reinterpret_cast<lor*>(e);
  }

  J_A(NODISC,RNN) land * expr_as_land(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::land);
    return reinterpret_cast<land*>(e);
  }

  J_A(NODISC,RNN) lnot * expr_as_lnot(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::lnot);
    return reinterpret_cast<lnot*>(e);
  }

  J_A(NODISC,RNN) to_bool * expr_as_to_bool(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::to_bool);
    return reinterpret_cast<to_bool*>(e);
  }

  J_A(NODISC,RNN) eq * expr_as_eq(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::eq);
    return reinterpret_cast<eq*>(e);
  }

  J_A(NODISC,RNN) neq * expr_as_neq(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::neq);
    return reinterpret_cast<neq*>(e);
  }

  J_A(NODISC,RNN) tag * expr_as_tag(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::tag);
    return reinterpret_cast<tag*>(e);
  }

  J_A(NODISC,RNN) untag * expr_as_untag(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::untag);
    return reinterpret_cast<untag*>(e);
  }

  J_A(NODISC,RNN) as_range * expr_as_as_range(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::as_range);
    return reinterpret_cast<as_range*>(e);
  }

  J_A(NODISC,RNN) as_vec * expr_as_as_vec(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::as_vec);
    return reinterpret_cast<as_vec*>(e);
  }

  J_A(NODISC,RNN) call * expr_as_call(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::call);
    return reinterpret_cast<call*>(e);
  }

  J_A(NODISC,RNN) full_call * expr_as_full_call(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::full_call);
    return reinterpret_cast<full_call*>(e);
  }

  J_A(NODISC,RNN) vec * expr_as_vec(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::vec);
    return reinterpret_cast<vec*>(e);
  }

  J_A(NODISC,RNN) make_vec * expr_as_make_vec(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::make_vec);
    return reinterpret_cast<make_vec*>(e);
  }

  J_A(NODISC,RNN) vec_append * expr_as_vec_append(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::vec_append);
    return reinterpret_cast<vec_append*>(e);
  }

  J_A(NODISC,RNN) vec_build * expr_as_vec_build(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::vec_build);
    return reinterpret_cast<vec_build*>(e);
  }

  J_A(NODISC,RNN) range_length * expr_as_range_length(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::range_length);
    return reinterpret_cast<range_length*>(e);
  }

  J_A(NODISC,RNN) range_get * expr_as_range_get(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::range_get);
    return reinterpret_cast<range_get*>(e);
  }

  J_A(NODISC,RNN) range_set * expr_as_range_set(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::range_set);
    return reinterpret_cast<range_set*>(e);
  }

  J_A(NODISC,RNN) range_copy * expr_as_range_copy(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::range_copy);
    return reinterpret_cast<range_copy*>(e);
  }

  J_A(NODISC,RNN) act_rec_alloc * expr_as_act_rec_alloc(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::act_rec_alloc);
    return reinterpret_cast<act_rec_alloc*>(e);
  }

  J_A(NODISC,RNN) act_rec_wr * expr_as_act_rec_wr(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::act_rec_wr);
    return reinterpret_cast<act_rec_wr*>(e);
  }

  J_A(NODISC,RNN) act_rec_rd * expr_as_act_rec_rd(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::act_rec_rd);
    return reinterpret_cast<act_rec_rd*>(e);
  }

  J_A(NODISC,RNN) act_rec_parent * expr_as_act_rec_parent(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::act_rec_parent);
    return reinterpret_cast<act_rec_parent*>(e);
  }

  J_A(NODISC,RNN) closure * expr_as_closure(expr * J_NOT_NULL e) noexcept {
    chk(e, expr_type::closure);
    return reinterpret_cast<closure*>(e);
  }
}
