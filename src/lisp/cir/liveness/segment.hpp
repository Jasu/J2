#pragma once

#include "lisp/cir/locs/loc_specifier.hpp"
#include "lisp/cir/ops/op_index.hpp"

namespace j::lisp::cir::inline liveness {
  struct segment final {
    op_index index_begin;
    op_index index_end;
    loc_specifier loc_specifier;
    loc loc = {};

    J_INLINE_GETTER u16_t size() const noexcept {
      return index_end.index - index_begin.index + 1U;
    }

    J_INLINE_GETTER bool contains(op_index idx) const noexcept {
      return index_begin <= idx && index_end >= idx;
    }
  };
}
