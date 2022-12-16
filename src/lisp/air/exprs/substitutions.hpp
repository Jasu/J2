#pragma once

#include "lisp/air/exprs/expr_action.hpp"

namespace j::lisp::air::exprs::substitutions {
  const inline substitution let_group[]{
    { "Merge (let (...) (let (...)))", merge_with(parent(expr_type::let), self()) },
  };
}
