#pragma once

#include "strings/string.hpp"

namespace j::strings {
  [[nodiscard]] inline i64_t string_cmp(const string & lhs, const string & rhs) noexcept {
    const u16_t lhs_tag = lhs.m_data;
    if (lhs_tag != (u16_t)rhs.m_data) {
      return lhs_tag < (u16_t)rhs.m_data ? -1 : 1;
    } else if (!(lhs_tag & 1U)) {
      return lhs.m_data - rhs.m_data;
    }
    return j::strcmp(reinterpret_cast<const char *>((lhs.m_data >> 16)),
                     reinterpret_cast<const char *>((rhs.m_data >> 16)));
  }

  struct string_comparer final {
    [[nodiscard]] i64_t operator()(const string & lhs, const_string_view rhs) const noexcept {
      const char * data = lhs.data();
      const u32_t sz = lhs.size();
      const i64_t result = ::j::memcmp(data, rhs.data(), ::j::min(sz, rhs.size()));
      return result ? result : sz - rhs.size();
    }

    [[nodiscard]] J_INLINE_GETTER i64_t operator()(const string & lhs, const string & rhs) const noexcept {
      return string_cmp(lhs, rhs);
    }
  };
}
