#pragma once

#include "hzd/string.hpp"
#include "hzd/crc32.hpp"
#include "strings/string.hpp"

namespace j::strings {
  J_A(ND,FLATTEN) inline u32_t hash_string(const char *J_AA(NN) begin, u32_t size) noexcept {
    return crc32(18U, begin, size);
  }

  template<typename T, auto V = T::npos>
  J_A(ND,FLATTEN) inline u32_t hash_string(const T & str) noexcept {
    return hash_string(str.data(), str.size());
  }

  J_A(ND,FLATTEN) inline u32_t hash_string(const char * const J_NOT_NULL s) noexcept {
    return hash_string(s, ::j::strlen(s));
  }

  struct string_hash {
    template<typename T, auto V = T::npos>
    J_A(ND,FLATTEN) inline u32_t operator()(const T & str) const noexcept {
      return hash_string(str.data(), str.size());
    }

    J_A(ND,FLATTEN) inline u32_t operator()(const char * const J_NOT_NULL s) const noexcept {
      return hash_string(s, ::j::strlen(s));
    }
  };
}
