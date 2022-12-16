#pragma once

#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::functions {
  struct func_wrapper final {
    lisp_fn_ptr_t fn = nullptr;

    J_INLINE_GETTER bool operator!() const noexcept {
      return !fn;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return fn;
    }

    lisp_imm operator()() const {
      return lisp_imm{fn(nullptr, 0)};
    }

    lisp_imm operator()(imms_t args) const {
      J_ASSUME_NOT_NULL(fn);
      return lisp_imm{fn(reinterpret_cast<u64_t*>(args.begin()), args.size())};
    }

    template<typename... Args>
    lisp_imm operator()(Args... args) const {
      J_ASSUME_NOT_NULL(fn);
      lisp_imm arg_array[]{args...};
      return lisp_imm{fn(reinterpret_cast<u64_t*>(arg_array), sizeof...(args))};
    }
  };
}
