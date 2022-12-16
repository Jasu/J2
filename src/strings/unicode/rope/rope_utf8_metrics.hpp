#pragma once

#include "hzd/utility.hpp"

namespace j::strings::inline unicode::inline rope {
  struct hard_line_breaks_key final {
    i64_t value;
    constexpr explicit hard_line_breaks_key(i64_t value) noexcept
      : value(value) { }
  };

  struct rope_utf8_metrics final {
    constexpr rope_utf8_metrics() noexcept = default;

    J_ALWAYS_INLINE constexpr rope_utf8_metrics(i64_t size_bytes,
                                             i64_t num_hard_breaks) noexcept
      : size_bytes(size_bytes), num_hard_breaks(num_hard_breaks)
    {
    }

    J_INLINE_GETTER constexpr explicit operator bool() const noexcept
    { return size_bytes || num_hard_breaks; }

    J_INLINE_GETTER constexpr i64_t key() const noexcept
    { return size_bytes; }

    J_INLINE_GETTER constexpr bool operator==(const rope_utf8_metrics & rhs) const noexcept = default;

    J_ALWAYS_INLINE constexpr rope_utf8_metrics & operator+=(const rope_utf8_metrics & rhs) noexcept {
      num_hard_breaks += rhs.num_hard_breaks;
      size_bytes += rhs.size_bytes;
      return *this;
    }

    J_ALWAYS_INLINE constexpr rope_utf8_metrics & operator-=(const rope_utf8_metrics & rhs) noexcept {
      num_hard_breaks -= rhs.num_hard_breaks;
      size_bytes -= rhs.size_bytes;
      return *this;
    }

    J_INLINE_GETTER constexpr rope_utf8_metrics operator-() const noexcept {
      return rope_utf8_metrics(-size_bytes, -num_hard_breaks);
    }

    J_INLINE_GETTER constexpr rope_utf8_metrics operator+(const rope_utf8_metrics & rhs) const noexcept {
      return rope_utf8_metrics(size_bytes + rhs.size_bytes,
                               num_hard_breaks + rhs.num_hard_breaks);
    }

    J_INLINE_GETTER constexpr rope_utf8_metrics operator-(const rope_utf8_metrics & rhs) const noexcept {
      return rope_utf8_metrics(size_bytes - rhs.size_bytes,
                               num_hard_breaks - rhs.num_hard_breaks);
    }

    i64_t size_bytes = 0;
    i64_t num_hard_breaks = 0;
  };
}
