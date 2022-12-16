#pragma once

#include "hzd/utility.hpp"

namespace j::tty {
  struct uv_termpaint_integration;

  enum class axis : u8_t {
    horizontal,
    vertical,
  };

  J_INLINE_GETTER axis orthogonal(axis a) noexcept {
    return a == axis::horizontal ? axis::vertical : axis::horizontal;
  }

  struct pos final {
    i16_t x = -1;
    i16_t y = -1;

    J_INLINE_GETTER bool operator==(const pos & rhs) const noexcept = default;

    pos & operator+=(const pos & rhs) noexcept {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    pos & operator-=(const pos & rhs) noexcept {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    pos operator+(const pos & rhs) noexcept {
      return {
        (i16_t)(x + rhs.x),
        (i16_t)(y + rhs.y),
      };
    }

    pos operator-(const pos & rhs) noexcept {
      return {
        (i16_t)(x - rhs.x),
        (i16_t)(y - rhs.y),
      };
    }

    pos operator-() const noexcept {
      return {(i16_t)-x, (i16_t)-y};
    }

    i16_t get(axis a) const noexcept {
      return a == axis::horizontal ? x : y;
    }

    void set(axis a, i16_t v) noexcept {
      (a == axis::horizontal ? x : y) = v;
    }

    void add(axis a, i16_t v) noexcept {
      (a == axis::horizontal ? x : y) += v;
    }
  };

  struct size final {
    i16_t width = -1;
    i16_t height = -1;

    bool operator==(const size & rhs) const noexcept = default;
    size min(const size & rhs) const noexcept {
      return {
        (i16_t)j::min((u16_t)width, (u16_t)rhs.width),
        (i16_t)j::min((u16_t)height, (u16_t)rhs.height)
      };
    }
    size max(const size & rhs) const noexcept {
      return {
        j::max(width, rhs.width),
        j::max(height, rhs.height)
      };
    }

    [[nodiscard]] i16_t get(axis a) const noexcept {
      return a == axis::horizontal ? width : height;
    }

    void set(axis a, i16_t sz) noexcept {
      (a == axis::horizontal ? width : height) = sz;
    }

    void add(axis a, i16_t v) noexcept {
      (a == axis::horizontal ? width : height) += v;
    }
  };

  enum class cursor_style : u8_t {
    hidden,
    normal,
    block,
    underline,
    bar,
  };
}
