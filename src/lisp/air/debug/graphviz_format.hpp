#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "graphviz/table.hpp"

namespace j::lisp::air::inline functions {
  struct air_function;
}
namespace j::lisp::air::exprs {
  struct expr;
}
namespace j::lisp::air::inline debug {
  [[nodiscard]] graphviz::table debug_to_table(const air_function * J_NOT_NULL fn,
                                               const exprs::expr * J_NOT_NULL expr, bool omit_result);
  [[nodiscard]] graphviz::table format_result_stub(const exprs::expr * J_NOT_NULL expr);
}
