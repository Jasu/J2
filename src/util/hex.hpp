#pragma once

#include "strings/string_view.hpp"

namespace j::util {
  J_A(AI,NODISC) inline constexpr bool is_hex_digit(char c) noexcept {
    return (c >= '0' && c <= '9')
      || (c >= 'a' && c <= 'f')
      || (c >= 'A' && c <= 'F');
  }

  [[nodiscard]] inline bool are_hex_digits(const strings::const_string_view & s) noexcept {
    bool result = true;
    for (auto c : s) {
      result &= is_hex_digit(c);
    }
    return result;
  }

  J_A(AI,NODISC) inline constexpr u8_t convert_hex_digit(char c) noexcept {
    return c > '9' ? (c | 0x20) - 'a' + 0xA : c - '0';
  }

  [[nodiscard]] inline constexpr u8_t convert_hex_byte(const char * J_NOT_NULL c) noexcept {
    return (convert_hex_digit(c[0]) << 4) | convert_hex_digit(c[1]);
  }

  [[nodiscard]] inline constexpr u16_t convert_hex_u16_t(const char * J_NOT_NULL c) noexcept {
    return ((u16_t)convert_hex_digit(c[0]) << 12)
         | ((u16_t)convert_hex_digit(c[1]) << 8)
         | ((u16_t)convert_hex_digit(c[2]) << 4)
         | (u16_t)convert_hex_digit(c[3]);
  }

  [[nodiscard]] inline constexpr u32_t convert_hex_u32_t(const char * J_NOT_NULL c) noexcept {
    return ((u32_t)convert_hex_digit(c[0]) << 28)
         | ((u32_t)convert_hex_digit(c[1]) << 24)
         | ((u32_t)convert_hex_digit(c[2]) << 20)
         | ((u32_t)convert_hex_digit(c[3]) << 16)
         | ((u32_t)convert_hex_digit(c[4]) << 12)
         | ((u32_t)convert_hex_digit(c[5]) << 8)
         | ((u32_t)convert_hex_digit(c[6]) << 4)
         | (u32_t)convert_hex_digit(c[7]);
  }
}
