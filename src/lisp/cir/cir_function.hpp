#pragma once

#include "lisp/compilation/const_table.hpp"
#include "containers/span.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"

namespace j::lisp::symbols {
  struct symbol;
}
namespace j::lisp::cir {
  struct cir_closure final {
    cir_ssa ssa_full_call;
    symbols::symbol * symbol = nullptr;
    compilation::const_table constants{};
  };

  struct cir_function final {
    i32_t num_closures = 0U;
    cir_ssa ssa_abi_call;
    cir_ssa ssa_full_call;

    span<cir_closure> closures() noexcept {
      return { reinterpret_cast<cir_closure*>(this + 1), num_closures };
    }

    span<const cir_closure> closures() const noexcept {
      return { reinterpret_cast<const cir_closure*>(this + 1), num_closures };
    }
  };
}
