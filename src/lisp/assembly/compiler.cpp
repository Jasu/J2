#include "lisp/assembly/compiler.hpp"

namespace j::lisp::assembly {
  compiler::compiler(function_builder * J_NOT_NULL builder) noexcept
    : builder(builder)
  { }

  compiler::~compiler() {
  }
}
