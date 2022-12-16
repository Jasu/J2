#pragma once

#include "debug/common.hpp"

#include "math/fixed.hpp"
#include "time/hrtime.hpp"

namespace j::debug {
  struct timer final {
    i64_t start_ts;
    i64_t sum_ts;
    u32_t count = 0U;
    u32_t start_count = 0U;
    J_ALWAYS_INLINE void start() noexcept {
      if (!(start_count++)) {
        start_ts = time::timer_at();
      }
    }
    J_ALWAYS_INLINE void stop() noexcept {
      if (!--start_count) {
        sum_ts += time::timer_at() - start_ts;
        ++count;
      }
    }
  };

  void register_timers(u32_t num,
                       timer * const * timers,
                       const char * group_name,
                       const char * names);


  struct J_TYPE_HIDDEN timer_registration final {
    template<typename... Ts>
    J_ALWAYS_INLINE explicit timer_registration(
      const char * group_name, const char * names, Ts & ... timers) noexcept {
      register_timers(sizeof...(timers), (timer*[]){&timers...}, group_name, names);
    }
  };

  void dump_timers();
}

#define J_DEFINE_TIMERS(GROUP_NAME, ...)                           \
  namespace j::_j_timers {                                         \
    inline ::j::debug::timer __VA_ARGS__;                          \
    inline const ::j::debug::timer_registration J_UNIQUE(_reg) {   \
      (GROUP_NAME), (#__VA_ARGS__), __VA_ARGS__};                  \
  }

#define J_START_TIMER(NAME) ::j::_j_timers::NAME.start()
#define J_STOP_TIMER(NAME) ::j::_j_timers::NAME.stop()

#define J_DUMP_TIMERS() ::j::debug::dump_timers()
