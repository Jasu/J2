#include "lisp/cir/target/cir_target.hpp"
#include "lisp/assembly/target.hpp"
#include "lisp/cir/target/amd64.hpp"

namespace j::lisp::cir {
  cir_target::cir_target(const assembly::target * J_NOT_NULL tgt) noexcept
    : reg_map(tgt),
      reg_info(tgt->get_reg_info()),
      ops(default_op_data)
  {
    initialize_target_amd64(*this);
  }
}
