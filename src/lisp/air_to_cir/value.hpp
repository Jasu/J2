#pragma once

#include "lisp/cir/ops/input.hpp"
#include "lisp/values/lisp_imms.hpp"


namespace j::lisp::air_to_cir {
  namespace c = cir;

  struct J_TYPE_HIDDEN value final {
    c::input value;
    c::input size;
  };
}
