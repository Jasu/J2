#pragma once

#include "colors/rgb.hpp"
#include "strings/string_view.hpp"

namespace j::graphviz {
  struct color final {
    J_A(AI,ND) constexpr inline color() noexcept = default;

    J_A(ND,AI) constexpr inline color(const char * J_NOT_NULL name) noexcept {
      __builtin_memcpy(value, name, 8U);
    }

    J_A(ND,AI) inline color(strings::const_string_view name) noexcept {
      __builtin_memcpy(value, name.data(), 8U);
    }

    J_A(ND,AI,HIDDEN) inline color & operator=(const char * J_NOT_NULL name) noexcept {
      __builtin_memcpy(value, name, 8U);
      return *this;
    }
    J_A(ND,AI,HIDDEN) inline color & operator=(strings::const_string_view name) noexcept {
      __builtin_memcpy(value, name.data(), 8U);
      return *this;
    }

    char value[8] = {0};

    color(colors::rgb8 color) noexcept;

    J_A(AI,HIDDEN) inline void reset() noexcept {
      j::memzero(value, 8);
    }

    J_A(AI,NODISC,HIDDEN,ND) inline explicit operator bool() const noexcept {
      return value[0];
    }

    J_A(AI,NODISC,HIDDEN,ND) inline bool operator!() const noexcept {
      return !value[0];
    }

    J_A(AI,NODISC,HIDDEN) inline strings::const_string_view format() const noexcept {
      return { value, value[0] ? 7 : 0 };
    }

    J_A(AI,NODISC,HIDDEN) inline bool operator==(const color &) const noexcept = default;
  };
}
