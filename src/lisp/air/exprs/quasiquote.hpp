#pragma once

#include "lisp/air/exprs/expr.hpp"
#include "containers/span.hpp"

namespace j::lisp::air::exprs {
  enum quasi_oper_type : u8_t {
    none = 0U,
    push,
    append,
  };

  struct quasi_operation final {
    quasi_oper_type type = quasi_oper_type::none;
    u32_t known_size = 0U;
  };

  using quasis_t = span<const quasi_operation>;
}
