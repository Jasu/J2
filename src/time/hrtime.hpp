#pragma once

#include "time/units.hpp"
#include "math/fixed.hpp"

namespace j::time {
  J_A(AI,ND,NODISC,FLATTEN,HIDDEN) inline u64_t timer_at() noexcept {
    return __rdtsc();
  }

  /// Get timer value in the unit specified.
  J_A(PURE,NODISC) math::s32_32 convert_timer_delta(i64_t timer, time_unit unit) noexcept;
}
