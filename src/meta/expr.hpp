#pragma once

#include "meta/errors.hpp"
#include "parsing/precedence_parser.hpp"
#include "meta/expr_fn.hpp"
#include "meta/attr_value.hpp"

namespace j::meta {
  struct module;
  struct dump_context;

  enum expr_type : u8_t {
    expr_none,

    expr_const,
    expr_var,

    expr_call,
    expr_method_call,

    expr_at_prop,
    expr_maybe_at_prop,
    expr_has,

    expr_is_none,
    expr_not,
    expr_neg,
    expr_bnot,

    expr_map,
    expr_reduce,
    expr_filter,
    expr_find,
    expr_find_index,
    expr_group_by,
    expr_sort_by,
    expr_every,
    expr_some,

    expr_or,
    expr_and,

    expr_bor,
    expr_band,

    expr_sub,
    expr_add,
    expr_mul,
    expr_div,

    expr_eq,
    expr_neq,
    expr_lt,
    expr_le,
    expr_gt,
    expr_ge,

    expr_if,
    expr_type_max = expr_if,
  };

  enum expr_form : u8_t {
    form_none,
    form_var,
    form_const,
    form_call,
    form_unary,
    form_binary,
    form_trinary,
  };

  struct expr;
  using dump_fn = void (*)(dump_context & ctx, const expr & e);

  struct expr_info final : parsing::operator_info {
    expr_form form = form_none;
    dump_fn dump = nullptr;
  };

  constexpr inline u8_t num_expr_types_v J_A(ND) = expr_type_max + 1U;

  extern constinit const expr_info exprs[num_expr_types_v];

  struct expr final {
    J_A(AI,ND) inline expr() noexcept : ops{nullptr} { }

    expr(const expr & rhs) noexcept;
    expr & operator=(const expr & rhs) noexcept;
    expr(expr && rhs) noexcept {
      ::j::memcpy(this, &rhs, sizeof(expr));
      ::new (&name) strings::string(static_cast<strings::string&&>(rhs.name));
      ::new (&idx_name) strings::string(static_cast<strings::string&&>(rhs.idx_name));
      rhs.type = expr_none;
    }

    expr & operator=(expr && rhs) noexcept;

    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return type != expr_none;
    }

    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return type == expr_none;
    }

    J_A(AI) inline ~expr() {
      if (type != expr_none) {
        clear();
      }
    }

    explicit expr(attr_value && const_val, source_location loc = {}) noexcept;

    void clear() noexcept;

    explicit expr(strings::const_string_view var_name, source_location loc = {}) noexcept;
    expr(strings::const_string_view fn_name, span<expr*> args, source_location loc = {}) noexcept;
    expr(strings::const_string_view fn_name, expr *this_arg, span<expr*> args, source_location loc = {}) noexcept;
    expr(expr_type type, strings::const_string_view it_name, strings::const_string_view idx_name,
         expr * base, expr * J_NOT_NULL body, expr * acc_init, source_location loc = {}) noexcept;
    expr(expr_type type, expr * base, strings::const_string_view at, source_location loc = {}) noexcept;

    J_A(ND) inline expr(expr_type type, expr * J_AA(NOALIAS) op0, expr * J_AA(NOALIAS) op1 = nullptr, expr * J_AA(NOALIAS) op2 = nullptr, source_location loc = {}) noexcept
    : type(type),
      ops{op0, op1, op2},
      loc(loc)
      { }

    J_A(ND) inline explicit expr(expr_type type, source_location loc = {}) noexcept
    : type(type),
      ops{nullptr},
      loc(loc)
      { }

    [[nodiscard]] bool operator==(const expr & rhs) const noexcept;
    [[nodiscard]] bool operator<(const expr & rhs) const noexcept;

    expr_type type = expr_none;
    strings::string name;
    strings::string idx_name;
    union {
      attr_value const_val;
      expr * ops[3];
      struct {
        expr ** args;
        u32_t size;
      } call;
    };
    source_location loc{};
  };

  struct expr_scope;

  [[nodiscard]] attr_value eval_expr(expr_scope & scope, const expr * J_NOT_NULL e);

  struct expr_hash final {
    J_A(FLATTEN,NODISC) u32_t operator()(const expr & part) const noexcept;
  };
}
