#pragma once

#include "lisp/air/values/rep.hpp"

namespace j::lisp::air::inline values {
  struct rep_counts final {
    u32_t tagged = 0U, untagged = 0U, range = 0U;

    inline void add(rep_spec spec, u8_t weight = 3U) noexcept {
      tagged += ((spec.has(rep_tagged) << 2) + spec.has_hint(rep_tagged)) << weight;
      untagged += ((spec.has(rep_untagged) << 2) + spec.has_hint(rep_untagged)) << weight;
      range += ((spec.has(rep_range) << 2) + spec.has_hint(rep_range)) << weight;
    }

    inline void add(rep spec, u8_t weight = 3U) noexcept {
      switch (spec) {
      case rep_tagged:
        tagged += 4U << weight;
        break;
      case rep_untagged:
        untagged += 4U << weight;
        break;
      case rep_range:
        range += 4U << weight;
        break;
      case rep_none:
        break;
      }
    }
    [[nodiscard]] inline rep get_max() const noexcept {
      return tagged < untagged
        ? (untagged < range ? rep_range : rep_untagged)
        : (tagged < range ? rep_range : rep_tagged);
    }

    J_A(AI) void reset() noexcept {
      tagged = untagged = range = 0U;
    }
  };
}
