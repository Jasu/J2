#pragma once

#include "lisp/cir_to_mc/mc_function.hpp"

namespace j::lisp::compilation {
  struct context;
}
namespace j::lisp::cir {
  struct cir_function;
}

namespace j::lisp::cir_to_mc {
  [[nodiscard]] mc_functions * cir_to_mc(compilation::context & ctx, cir::cir_function * J_NOT_NULL cir_fn);
}
