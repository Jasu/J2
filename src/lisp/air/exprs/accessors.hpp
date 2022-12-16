#pragma once

#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/air_var.hpp"

namespace j::lisp::air::exprs {
  J_A(NODISC) inline air_var & get_lex_var(const expr * J_NOT_NULL e) noexcept {
    return **reinterpret_cast<air_var * const *>(e->static_begin());
}

template<bool IsConst, expr_type... Types>
struct lex_accessor {
  const_if_t<IsConst, exprs::expr> & e;

  lex_accessor(const_if_t<IsConst, exprs::expr> & e) : e(e)
  { J_ASSERT(((e.type == Types) || ...)); }

  [[nodiscard]] auto & var() const noexcept {
    return *reinterpret_cast<const_if_t<IsConst, air_var*>*>(e.static_begin());
  }
};

template<bool IsConst>
struct lex_rd_accessor final : lex_accessor<IsConst, expr_type::lex_rd, expr_type::lex_rd_bound> {
  using lex_accessor<IsConst, expr_type::lex_rd, expr_type::lex_rd_bound>::lex_accessor;

  J_INLINE_GETTER decltype(auto) reaching_writes() const noexcept {
    return *add_bytes<span<const_if_t<IsConst, expr*>>*>(
      lex_accessor<IsConst, expr_type::lex_rd, expr_type::lex_rd_bound>::e.static_begin(), sizeof(air_var*));
    }
  };

  template<bool IsConst>
  struct lex_wr_accessor final : lex_accessor<IsConst, expr_type::lex_wr, expr_type::lex_wr_bound> {
    using lex_accessor<IsConst, expr_type::lex_wr, expr_type::lex_wr_bound>::lex_accessor;

    J_INLINE_GETTER decltype(auto) reads() const noexcept {
      return *add_bytes<span<const_if_t<IsConst, expr*>>*>(
        lex_accessor<IsConst, expr_type::lex_wr, expr_type::lex_wr_bound>::e.static_begin(), sizeof(air_var*));
    }
  };

  lex_rd_accessor(const exprs::expr &) -> lex_rd_accessor<true>;
  lex_rd_accessor(exprs::expr &) -> lex_rd_accessor<false>;
  lex_wr_accessor(const exprs::expr &) -> lex_wr_accessor<true>;
  lex_wr_accessor(exprs::expr &) -> lex_wr_accessor<false>;

}
