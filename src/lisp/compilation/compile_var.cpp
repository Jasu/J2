#include "lisp/compilation/compile_var.hpp"

#include "lisp/values/lisp_imms.hpp"
#include "lisp/symbols/symbol.hpp"
#include "lisp/compilation/context.hpp"

namespace j::lisp::compilation {
  void compile_var(context & c) {
    c.symbol.value = c.symbol.value.as_vec_ref()->at(2);
  }
}
