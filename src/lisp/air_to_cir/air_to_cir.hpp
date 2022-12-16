#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::compilation {
  struct context;
}

namespace j::lisp::air::inline functions {
  struct air_function;
}

namespace j::lisp::cir {
  struct cir_function;
}

namespace j::lisp::air_to_cir {
  [[nodiscard]] J_RETURNS_NONNULL cir::cir_function * air_to_cir(
    compilation::context & ctx,
    air::air_function * J_NOT_NULL air_fn);
}
