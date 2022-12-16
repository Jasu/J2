#pragma once

#include "hzd/mem.hpp"

namespace j {
  J_A(AI,NODISC,FLATTEN,ND,MU) static inline constexpr const char * strchr(const char * J_NOT_NULL str, char chr) noexcept {
    return __builtin_strchr(str, chr);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline constexpr char * strchr(char * J_NOT_NULL str, char chr) noexcept {
    return __builtin_strchr(str, chr);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline const char * strrchr(const char * J_NOT_NULL str, char chr) noexcept {
    return __builtin_strrchr(str, chr);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline char * strrchr(char * J_NOT_NULL str, char chr) noexcept {
    return __builtin_strrchr(str, chr);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline sz_t strspn(const char * J_NOT_NULL str, const char * J_NOT_NULL mask) noexcept {
    return __builtin_strspn(str, mask);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline sz_t strcspn(const char * J_NOT_NULL str, const char * J_NOT_NULL mask) noexcept {
    return __builtin_strcspn(str, mask);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline constexpr const char * memchr(const char * J_NOT_NULL start, char chr, sz_t size) noexcept {
    return __builtin_char_memchr(start, chr, size);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline constexpr char * memchr(char * J_NOT_NULL start, char chr, sz_t size) noexcept {
    return __builtin_char_memchr(start, chr, size);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline constexpr u32_t strlen(const char * J_NOT_NULL str) noexcept {
    return __builtin_strlen(str);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline i32_t memcmp(const void * a, const void * b, sz_t size) noexcept {
    return __builtin_memcmp(a, b, size);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline i32_t strcmp(const char * J_NOT_NULL a, const char * J_NOT_NULL b) noexcept {
    return __builtin_strcmp(a, b);
  }

  J_A(AI,NODISC,FLATTEN,ND,MU) static inline i32_t strncmp(const char * J_NOT_NULL a, const char * J_NOT_NULL b, sz_t sz) noexcept {
    return __builtin_strncmp(a, b, sz);
  }
}
