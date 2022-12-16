#pragma once

#include "lisp/cir/ops/op.hpp"

namespace j::lisp::cir::inline ops {
  struct arg_accessor final {
    const op & o;

    J_ALWAYS_INLINE explicit arg_accessor(const op & o) noexcept
      : o(o)
    {
      J_ASSUME(o.type == op_type::fn_iarg || o.type == op_type::fn_farg);
    }

    J_INLINE_GETTER u32_t index() const noexcept {
      return *reinterpret_cast<const u32_t*>(o.static_begin());
    }

    J_INLINE_GETTER bool has_rest_arg() const noexcept {
      return *reinterpret_cast<const bool*>(add_bytes(o.static_begin(), sizeof(u32_t)));
    }

    J_INLINE_GETTER bool has_static_chain() const noexcept {
      return *reinterpret_cast<const bool*>(add_bytes(o.static_begin(), sizeof(u32_t) + sizeof(bool)));
    }
  };

  J_INLINE_GETTER arg_accessor as_arg(const op & op) noexcept {
    return arg_accessor(op);
  }
}
