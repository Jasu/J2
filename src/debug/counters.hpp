#pragma once

#include "debug/common.hpp"

namespace j::debug {
  void register_counters(u32_t num, const u32_t * const * counters,
                         const char * group_name, const char * names) noexcept;

  struct J_TYPE_HIDDEN counter_registration final {
    template<typename... Ts>
    J_ALWAYS_INLINE_NO_DEBUG explicit counter_registration(const char * group_name, const char * names, const Ts & ... counters) noexcept {
      register_counters(sizeof...(counters), (const u32_t*[]){&counters...}, group_name, names);
    }
  };

  void dump_counters(u32_t flags = accumulate | only_changed);
}

#ifndef NDEBUG

#define J_DEFINE_COUNTERS(GROUP_NAME, ...)                         \
  namespace j::_j_counters {                                       \
    inline u32_t __VA_ARGS__;                                      \
    inline const ::j::debug::counter_registration J_UNIQUE(_reg) { \
      (GROUP_NAME), (#__VA_ARGS__), __VA_ARGS__};                  \
  }

#define J_COUNT_IMPL(N, V, ...) (void)(::j::_j_counters::N += (V))
#define J_COUNT(...) J_COUNT_IMPL(__VA_ARGS__, 1U)

#define J_DUMP_COUNTERS(...) do { using namespace ::j::debug; ::j::debug::dump_counters(__VA_ARGS__); } while (false)

#else

#define J_DEFINE_COUNTERS(...)
#define J_COUNT(...) do { } while (false)
#define J_DUMP_COUNTERS(...) do { } while (false)

#endif
