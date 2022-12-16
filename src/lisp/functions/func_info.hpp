#pragma once

#include "lisp/common/id.hpp"
#include "lisp/mem/code_heap/allocation.hpp"
#include "containers/span.hpp"
#include "lisp/functions/parameters.hpp"
#include "lisp/functions/calling_convention.hpp"

namespace j::lisp::assembly::inline functions {
  struct func_info;
}

namespace j::lisp::symbols {
  struct symbol;
}

namespace j::lisp::functions {
  struct func_cc_info final {
    void * fn_ptr = nullptr;
    assembly::func_info * asm_info = nullptr;
    mem::code_heap::allocation allocation;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return fn_ptr;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !fn_ptr;
    }
  };

  struct closure_info final {
    symbols::symbol * sym = nullptr;
    cc_map<func_cc_info> cc_info;
  };


  /// Information about a top-level function, a lambda function, or the pkg `:load` function.
  struct func_info final {
    cc_map<func_cc_info> cc_info;

    span<closure_info> closures;

    parameters params;

    J_ALWAYS_INLINE id param(u32_t i) const noexcept {
      return params[i];
    }
  };
}
