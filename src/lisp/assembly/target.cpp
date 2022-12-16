#include "lisp/assembly/amd64/abi/ia64_target.hpp"

namespace j::lisp::assembly {
  J_A(RNN,NODISC) const target * target::get_target(target_name name) noexcept {
    switch (name) {
    case target_name::amd64_ia64: return &amd64::abi::ia64_target::instance;
    }
  }

  target::~target() {
  }
}
