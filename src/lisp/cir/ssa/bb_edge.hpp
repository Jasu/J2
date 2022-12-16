#pragma once

#include "hzd/types.hpp"

namespace j::lisp::cir::inline ssa {
  /// A branch between two basic blocks.
  struct bb_edge final {
    /// The basic block the edge is going to / coming from.
    struct bb * bb = nullptr;
    /// Index of the edge in the source / target basic block.
    u8_t index = 0U;
  };
}
