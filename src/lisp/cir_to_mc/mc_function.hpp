#pragma once

#include "lisp/assembly/functions/function.hpp"
#include "lisp/functions/calling_convention.hpp"

namespace j::lisp::symbols {
  struct symbol;
}

namespace j::lisp::cir_to_mc {
  struct mc_closure final {
    symbols::symbol * symbol = nullptr;
    functions::cc_map<j::lisp::assembly::function> asm_funcs;
    J_INLINE_GETTER explicit operator bool() const noexcept {
      return symbol;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !symbol;
    }
  };

  struct mc_functions final {
    functions::cc_map<j::lisp::assembly::function> asm_funcs;
    i32_t num_closures = 0;
    J_INLINE_GETTER span<mc_closure> closures() noexcept {
      return { reinterpret_cast<mc_closure*>(this + 1), num_closures };
    }
    J_INLINE_GETTER const span<const mc_closure> closures() const noexcept {
      return { reinterpret_cast<const mc_closure*>(this + 1), num_closures };
    }
  };
}
