#pragma once

#include "lisp/cir/passes/cir_pass.hpp"

namespace j::lisp::cir::inline liveness {
  extern const cir_pass compute_live_ranges_pass;
}
namespace j::lisp::cir::reg_alloc {
  extern const cir_pass allocate_regs_pass;
  extern const cir_pass resolve_phis_pass;
}
namespace j::lisp::cir::inline passes {
  extern const cir_pass reverse_postorder_pass;
  extern const cir_pass number_ops_pass;
  extern const cir_pass full_call_pass;
  extern const cir_pass def_use_pass;
  extern const cir_pass legalize_pass;
  extern const cir_pass remove_empty_bbs_pass;
}
