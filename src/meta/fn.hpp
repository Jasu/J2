#pragma once

#include "meta/node.hpp"

namespace j::meta {
  struct expr;
  struct expr_fn;

  struct fn final : node {
    J_BOILERPLATE(fn, CTOR_NE_ND)
    fn(strings::const_string_view name, doc_comment && comment) noexcept;

    expr_fn to_expr_fn() const noexcept;

    void push_arg(strings::const_string_view name, bool is_rest);

    strings::string args[4U];
    expr * e = nullptr;
    u8_t num_args = 0U;
    bool has_rest = false;
  };
}
