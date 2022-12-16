#pragma once

#include "hzd/utility.hpp"

namespace j {
  J_A(ND,MU,HIDDEN,AI,NODISC) static inline bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
  }

  J_A(ND,MU,HIDDEN,AI,NODISC) static inline bool is_alpha(char c) noexcept {
    c |= 0x20;
    return c >= 'a' && c <= 'z';
  }

  J_A(ND,MU,HIDDEN,AI,NODISC) static inline bool is_lower(char c) noexcept {
    return c >= 'a' && c <= 'z';
  }

  J_A(ND,MU,HIDDEN,AI,NODISC) static inline bool is_upper(char c) noexcept {
    return c >= 'A' && c <= 'Z';
  }

  J_A(ND,MU,HIDDEN,AI,NODISC,FLATTEN) static inline bool is_alnum(char c) noexcept {
    return is_digit(c) || is_alpha(c);
  }
}
