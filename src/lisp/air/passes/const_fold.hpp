#pragma once

#include "lisp/air/passes/air_passes.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "lisp/air/exprs/expr_type.hpp"

namespace j::lisp::packages {
  struct pkg;
}
namespace j::lisp::env {
  struct environment;
}

namespace j::lisp::air::inline passes {
  struct const_folder final {
    lisp_abi_call_1_ptr_t fn_1 = nullptr;
    lisp_abi_call_2_ptr_t fn_2 = nullptr;
    lisp_abi_call_2_ptr_t fn_tail = nullptr;

    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return fn_2 || fn_1 || fn_tail;
    }
    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return !fn_2 && !fn_1 && !fn_tail;
    }
    J_A(AI,NODISC) inline bool is_foldable_unary() const noexcept {
      return fn_1;
    }
    J_A(AI,NODISC) inline bool is_foldable_binary() const noexcept {
      return fn_2;
    }
    J_A(AI,NODISC) inline bool is_tail_foldable() const noexcept {
      return fn_2 || fn_tail;
    }

    J_A(AI,NODISC) inline lisp_imm fold(lisp_imm arg) const noexcept {
      J_ASSERT_NOT_NULL(fn_1);
      return lisp_imm{fn_1(arg.raw)};
    }

    J_A(AI,NODISC) inline lisp_imm fold(lisp_imm lhs, lisp_imm rhs) const noexcept {
      J_ASSERT_NOT_NULL(fn_2);
      return lisp_imm{fn_2(lhs.raw, rhs.raw)};
    }

    J_A(AI,NODISC) inline lisp_imm fold_tail(lisp_imm lhs, lisp_imm rhs) const noexcept {
      J_ASSERT(fn_tail || fn_2);
      return lisp_imm{(fn_tail ? fn_tail : fn_2)(lhs.raw, rhs.raw)};
    }
  };

  struct const_fold_table final {
    const_folder folders[num_expr_types_v] = {};

    J_A(AI,NODISC) inline lisp_imm fold_tail(expr_type t, lisp_imm lhs, lisp_imm rhs) const noexcept {
      return folders[(u8_t)t].fold_tail(lhs, rhs);
    }

    J_A(AI,NODISC) inline lisp_imm fold(expr_type t, lisp_imm lhs, lisp_imm rhs) const noexcept {
      return folders[(u8_t)t].fold(lhs, rhs);
    }

    J_A(AI,NODISC) inline lisp_imm fold(expr_type t, lisp_imm val) const noexcept {
      return folders[(u8_t)t].fold(val);
    }

    J_A(AI,NODISC) inline bool is_foldable_unary(expr_type t) const noexcept {
      return folders[(u8_t)t].is_foldable_unary();
    }
    J_A(AI,NODISC) inline bool is_foldable_binary(expr_type t) const noexcept {
      return folders[(u8_t)t].is_foldable_binary();
    }
    J_A(AI,NODISC) inline bool is_tail_foldable(expr_type t) const noexcept {
      return folders[(u8_t)t].is_tail_foldable();
    }

    J_A(AI,NODISC) inline const const_folder & operator[](expr_type t) const noexcept {
      return folders[(u8_t)t];
    }
  };

  J_A(RNN,NODISC) packages::pkg * create_const_fold_pkg(env::environment * J_NOT_NULL e);
  J_A(RNN,NODISC) const const_fold_table * get_const_fold_table(env::environment * J_NOT_NULL e);
}
