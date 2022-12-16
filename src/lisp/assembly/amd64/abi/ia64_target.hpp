#pragma once

#include "lisp/assembly/amd64/amd64_target.hpp"

namespace j::lisp::assembly::amd64::abi {
  class ia64_target final : public amd64_target {
  public:
    J_A(RNN,NODISC) const reg_info * get_reg_info() const noexcept override;

    J_A(ND,NODESTROY) static const ia64_target instance;
  };
}
