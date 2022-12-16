#pragma once

#include "hzd/utility.hpp"

namespace j::time {
  enum class time_unit : u8_t {
    ns,
    us,
    ms,
    s,
    m,
    h,
    max = h,
  };

  constexpr inline u8_t num_time_units_v = 1U + (u8_t)time_unit::max;

  constexpr inline i64_t time_unit_factors[num_time_units_v][num_time_units_v]{
    // To:             ns              µs           ms        s            min           hour
    /* From:   ns */ { 1L,             1000L,       1000000L, 1000000000L, 60000000000L, 3600000000000L },
    /* From:   µs */ { 1000L,          1L,          1000L,    1000000L,    60000000L,    3600000000L    },
    /* From:   ms */ { 1000000L,       1000L,       1L,       1000L,       60000L,       3600000L       },
    /* From:    s */ { 1000000000L,    1000000L,    1000L,    1L,          60L,          3600L          },
    /* From:  min */ { 60000000000L,   60000000L,   60000L,   60L,         1L,           60L            },
    /* From: hour */ { 3600000000000L, 3600000000L, 3600000L, 3600L,       60L,          1L             },
  };

  template<typename Num>
  [[nodiscard]] inline constexpr Num time_convert(Num t, time_unit from, time_unit to) noexcept {
    i64_t conv = time_unit_factors[(u8_t)from][(u8_t)to];
    return from <= to ? t * conv : t / conv;
  }
}
