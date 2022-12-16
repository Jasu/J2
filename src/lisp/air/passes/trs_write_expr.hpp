#pragma once
// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/air/exprs/write_utils.hpp"
#include "lisp/air/exprs/expr.hpp"

namespace j::lisp::air::inline functions { struct air_closure; }
namespace j::mem { class bump_pool; }
namespace j::lisp::air::exprs {
  struct quasi_operation;
  using quasis_t = span<const quasi_operation>;
}
namespace j::lisp::air::inline passes {
  using exprs::expr;
  using exprs::vars_init;
  using exprs::input;
  using exprs::quasis_t;

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, air_closure* act_rec);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, air_var* var);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, id arg0);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, lisp_imm const_val);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, imm_type_mask value);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, u32_t index);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, quasis_t elems);
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs, vars_init forms);

  J_A(AI,ND,HIDDEN,NODISC) inline i32_t get_in_size(expr * e) noexcept {
    return e ? 1 : 0;
  }

  J_A(AI,ND,HIDDEN,NODISC) inline i32_t get_in_size(span<expr*> s) noexcept {
    return s.size();
  }

  J_A(AI,ND,HIDDEN) inline void copy_in(expr * J_NOT_NULL parent, i32_t & index, expr * e) noexcept {
    if (e) {
      parent->set_input(index++, e);
    }
  }

  J_A(AI,ND,HIDDEN) inline void copy_in(expr * J_NOT_NULL parent, i32_t & index, span<expr*> s) noexcept {
    for (auto e : s) {
      J_ASSERT_NOT_NULL(e);
      parent->set_input(index++, e);
    }
  }

  template<typename Arg>
  struct J_TYPE_HIDDEN const_arg_wrapper final {
    using type = const Arg &;
  };

  struct J_TYPE_HIDDEN const_id_wrapper final {
    J_A(AI,ND) inline const_id_wrapper(id i) noexcept : value(i) { }
    J_A(AI,ND) inline const_id_wrapper(u32_t i) noexcept : value(lisp::raw_tag, i) { }
    J_A(AI,ND,NODISC) inline operator id() const noexcept { return value; }
    id value;
  };

  template<>
  struct J_TYPE_HIDDEN const_arg_wrapper<id> final {
    using type = const_id_wrapper;
  };

  template<typename... ConstArgs, typename... Ins>
  J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * trs_wr(expr_type type, j::mem::bump_pool & pool,
                                                    typename const_arg_wrapper<ConstArgs>::type ... const_args, const Ins & ... ins) {
    i32_t in_size = (get_in_size(ins) + ... + 0);
    expr * result = trs_construct(type, pool, in_size, const_args...);
    if constexpr (sizeof...(Ins) > 0) {
      i32_t index = 0;
      (copy_in(result, index, ins), ...);
    }
    return result;
  }

  struct J_TYPE_HIDDEN trs_writer final {
    j::mem::bump_pool & pool;
    template<typename... ConstArgs, typename... Ins>
    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * wr(expr_type type, typename const_arg_wrapper<ConstArgs>::type... const_args, const Ins & ... ins) {
      return trs_wr<ConstArgs...>(type, pool, const_args..., ins...);
    }

    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * wrc(lisp_imm val) {
      return trs_construct(expr_type::ld_const, pool, 0, val);
    }

    template<typename Num>
    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * i64(Num num) {
      return trs_construct(expr_type::ld_const, pool, 0, lisp_imm{(u64_t)num << 1});
    }

    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * id(u64_t num) {
      return trs_construct(expr_type::ld_const, pool, 0, lisp_imm{num});
    }

    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * id(lisp::id i) {
      return trs_construct(expr_type::ld_const, pool, 0, lisp_imm{i});
    }

    template<typename Id, typename Imm>
    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * sym_wr(Id id, Imm value) {
      return trs_wr<lisp::id>(expr_type::sym_val_wr, pool, id, value);
    }

    template<typename... Ins>
    J_A(RNN,NODISC,AI,ND,HIDDEN) inline expr * progn(const Ins & ... ins) {
      return trs_wr<>(expr_type::progn, pool, ins...);
    }
  };
}
