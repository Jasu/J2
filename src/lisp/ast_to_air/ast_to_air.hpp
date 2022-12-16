#pragma once

#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::air::inline functions {
  struct air_function;
}
namespace j::lisp::compilation {
  struct context;
}
namespace j::lisp::ast_to_air {
  [[nodiscard]] J_RETURNS_NONNULL air::air_function * top_level_defun_to_air(compilation::context & ctx, imms_t body);
}
