#pragma once

#include "lisp/cir/ops/op_data.hpp"
#include "lisp/cir/locs/reg_map.hpp"

namespace j::lisp::assembly {
  class target;
  struct reg_info;
}

namespace j::lisp::cir  {
  struct cir_target final {
    explicit cir_target(const assembly::target * J_NOT_NULL tgt) noexcept;

    reg_map reg_map;
    const assembly::reg_info * reg_info;
    ops::op_data_table ops;
  };
}
