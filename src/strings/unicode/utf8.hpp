#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::strings::inline unicode {
  /// Code point of zero-width joiner.
  J_A(ND,MU) inline static constexpr u32_t code_point_zwj = 0x200DU;

  /// Code point of the replacement character.
  J_A(ND,MU) inline static constexpr u32_t code_point_replacement_character = 0xFFFDU;

  J_A(AI,ND,NODISC,HIDDEN) inline bool utf8_is_successor_byte(u8_t c) noexcept {
    return (c & 0b11000000) == 0b10000000;
  }
  J_A(AI,ND,NODISC,HIDDEN) inline bool utf8_is_successor_byte(char c) noexcept {
    return utf8_is_successor_byte((u8_t)c);
  }

  J_INLINE_GETTER constexpr u8_t utf8_leading_byte_code_point_bytes(u8_t c) noexcept {
    return c < 0xE0
      ? (c < 0xC2 ? 1U : 2U)
      : (c < 0xF0 ? 3U : (J_UNLIKELY(c > 0xF4) ? 1U : 4U));
  }

  J_INLINE_GETTER constexpr u8_t utf8_leading_byte_code_point_bytes(char c) noexcept {
    return utf8_leading_byte_code_point_bytes((u8_t)c);
  }

  /// Get bytes consumed by the code point starting at c.
  ///
  /// \param c Pointer to the start of the code point. The pointer must be followed
  ///          by the complete code unit sequence - `utf8_code_point_bytes` attempts
  ///          to read the whole code unit sequence to validate the encoding.
  ///
  /// \return Number of bytes consumed by the code point at c. An integer in range [1, 4].
  ///         If the code unit sequence starting at c is ill-formed, the length of the
  ///         maximal subpart of the ill formed sequence (D93b) is returned.
  ///         In other words, length of the valid portion of the sequence is returned, or
  ///         if the leading character itself is invalid, one is returned.
  [[nodiscard]] inline u8_t utf8_code_point_bytes(const u8_t * J_NOT_NULL const c) noexcept {
    const u8_t result = utf8_leading_byte_code_point_bytes(*c);
    for (u8_t i = 1; i < result; ++i) {
      if (J_UNLIKELY(!utf8_is_successor_byte(c[i]))) {
        return i;
      }
    }
    return result;
  }

  [[nodiscard]] inline u8_t utf8_code_point_bytes(const u8_t * J_NOT_NULL const c,
                                                  const u8_t * J_NOT_NULL const end) noexcept {
    J_ASSUME(c < end);
    const u8_t result = ::j::min(utf8_leading_byte_code_point_bytes(*c), end - c);
    for (u8_t i = 1; i < result; ++i) {
      if (J_UNLIKELY(!utf8_is_successor_byte(c[i]))) {
        return i;
      }
    }
    return result;
  }

  J_INLINE_GETTER u8_t utf8_code_point_bytes(const char * J_NOT_NULL const c) noexcept {
    return utf8_code_point_bytes((const u8_t*)c);
  }

  J_INLINE_GETTER u8_t utf8_code_point_bytes(const char * J_NOT_NULL const c,
                                             const char * J_NOT_NULL const end) noexcept {
    return utf8_code_point_bytes((const u8_t*)c, (const u8_t*)end);
  }

  [[nodiscard]] inline bool utf8_is_code_point_valid(const u8_t * J_NOT_NULL const c, u32_t sz) noexcept {
    J_ASSUME(sz > 0);
    const u8_t result = utf8_leading_byte_code_point_bytes(*c);
    if (result > sz) {
      return false;
    }
    for (u8_t i = 1; i <= result; ++i) {
      if (!utf8_is_successor_byte(c[i])) {
        return false;
      }
    }
    return true;
  }

  J_INLINE_GETTER bool utf8_is_code_point_valid(const char * J_NOT_NULL const c, u32_t sz) noexcept {
    return utf8_is_code_point_valid((const u8_t*)c, sz);
  }

  /// Get bytes consumed by the code point ending at c.
  ///
  /// \param c Pointer to the last byte of the code point. The pointer must be preceded
  ///          by the complete code unit sequence - `utf8_code_point_bytes_backwards` has
  ///          to read up to 3 bytes backwards.
  ///
  /// \return The number of bytes consumed by the code point at c. An integer in range [1, 4].
  [[nodiscard]] inline u32_t utf8_code_point_bytes_backwards(const u8_t * J_NOT_NULL c) noexcept {
    u32_t num_successors = 0;
    for (; num_successors < 3;) {
      if (!utf8_is_successor_byte(*c)) {
        break;
      }
      --c, ++num_successors;
    }
    if (J_LIKELY(num_successors == 0)) {
      return 1;
    }
    const u32_t expected_successors = utf8_leading_byte_code_point_bytes(*c) - 1U;
    return J_LIKELY(num_successors <= expected_successors)
      ? num_successors + 1
      : 1U;
  }

  J_INLINE_GETTER u32_t utf8_code_point_bytes_backwards(const char * J_NOT_NULL c) noexcept {
    return utf8_code_point_bytes_backwards((const u8_t*)c);
  }

  /// Check whether the code point starting at c is truncated.
  [[nodiscard]] inline u32_t utf8_code_point_truncated_by(const u8_t * J_NOT_NULL c) noexcept {
    u32_t len = utf8_leading_byte_code_point_bytes(*c) - 1U;
    if (len && utf8_is_successor_byte(*++c)) { --len; }
    if (len && utf8_is_successor_byte(*++c)) { --len; }
    if (len && utf8_is_successor_byte(*++c)) { --len; }
    return len;
  }

  J_INLINE_GETTER u32_t utf8_code_point_truncated_by(const char * J_NOT_NULL c) noexcept {
    return utf8_code_point_truncated_by((const u8_t *)c);
  }

  /// Check whether the code point starting at c is truncated.
  [[nodiscard]] inline u32_t utf8_code_point_truncated_by_backwards(const u8_t * J_NOT_NULL c) noexcept {
    if (!utf8_is_successor_byte(*c)) {
      return utf8_leading_byte_code_point_bytes(*c) - 1U;
    }
    if (!utf8_is_successor_byte(*--c)) {
      const u32_t b = utf8_leading_byte_code_point_bytes(*c);
      return b > 2U ? b - 2U : 0U;
    }
    if (!utf8_is_successor_byte(*--c)) {
      const u32_t b = utf8_leading_byte_code_point_bytes(*c);
      return b == 4U ? 1U : 0U;
    }
    return 0U;
  }

  J_INLINE_GETTER u32_t utf8_code_point_truncated_by_backwards(const char * J_NOT_NULL c) noexcept {
    return utf8_code_point_truncated_by_backwards((const u8_t *)c);
  }

  [[nodiscard]] inline u32_t utf8_num_leading_successors(const u8_t * J_NOT_NULL const c, const u32_t sz) noexcept {
    J_ASSUME(sz > 0);
    u32_t num = 0U;
    for (; num < sz && utf8_is_successor_byte(c[num]); ++num) { }
    return num;
  }

  J_INLINE_GETTER u32_t utf8_num_leading_successors(const char * J_NOT_NULL const c, const u32_t sz) noexcept {
    return utf8_num_leading_successors((const u8_t*)c, sz);
  }

  [[nodiscard]] inline i32_t utf8_num_trailing_successors(const u8_t * J_NOT_NULL c, const u32_t sz) noexcept {
    J_ASSUME(sz > 0);
    u32_t num = 0U;
    for (; num < sz; ++num, --c) {
      if (!utf8_is_successor_byte(*c)) {
        return num - utf8_leading_byte_code_point_bytes(*c) + 1;
      }
    }
    return num;
  }

  J_INLINE_GETTER i32_t utf8_num_trailing_successors(const char * J_NOT_NULL const c, const u32_t sz) noexcept {
    return utf8_num_trailing_successors((const u8_t*)c, sz);
  }

  [[nodiscard]] inline u32_t utf8_get_code_point(const u8_t * J_NOT_NULL const c) noexcept {
    const u32_t first = *c;
    if (first < 0x80) { return *c; }
    u32_t succ1 = c[1] - 0b10000000;
    if (J_UNLIKELY(succ1 > 0b00111111)) { return 0xFFFDU; }
    if (first < 0xE0) {
      if (J_UNLIKELY(first < 0xC2)) { return code_point_replacement_character; }
      return ((first & 0b00011111U) << 6U) | succ1;
    }
    u32_t succ2 = c[2] - 0b10000000;
    if (J_UNLIKELY(succ2 > 0b00111111)) { return code_point_replacement_character; }
    if (first < 0xF0) {
      if (J_UNLIKELY((first == 0xE0U && succ1 < 0x20U) || (first == 0xEDU && succ1 >= 0x20U))) {
        return code_point_replacement_character;
      }
      return ((first & 0b00001111U) << 12U) | (succ1 << 6U) | succ2;
    }
    u32_t succ3 = c[3] - 0b10000000;
    if (J_UNLIKELY(succ3 > 0b00111111 || first > 0xF4 || (first == 0xF0 && succ1 < 0x10) || (first == 0xF4 && succ1 > 0xF))) {
      return code_point_replacement_character;
    }
    return ((first & 0b00000111U) << 18U) | (succ1 << 12U) | (succ2 << 6U) | succ3;
  }

  [[nodiscard]] inline u32_t utf8_get_code_point(const u8_t * J_NOT_NULL const c,
                                            const u8_t * J_NOT_NULL const end) noexcept {
    const u32_t first = *c;
    if (first < 0x80) { return first; }
    const u32_t sz = end - c;
    if (J_UNLIKELY(sz == 1U)) { return code_point_replacement_character; }
    u32_t succ1 = c[1] - 0b10000000;
    if (J_UNLIKELY(succ1 > 0b00111111)) { return code_point_replacement_character; }
    if (first < 0xE0) {
      if (J_UNLIKELY(first < 0xC2)) { return code_point_replacement_character; }
      return ((first & 0b00011111U) << 6U) | succ1;
    }
    if (J_UNLIKELY(sz == 2U)) { return code_point_replacement_character; }
    u32_t succ2 = c[2] - 0b10000000;
    if (J_UNLIKELY(succ2 > 0b00111111)) { return code_point_replacement_character; }
    if (first < 0xF0) {
      if (J_UNLIKELY((first == 0xE0U && succ1 < 0x20U) || (first == 0xEDU && succ1 >= 0x20U))) {
        return code_point_replacement_character;
      }
      return ((first & 0b00001111U) << 12U) | (succ1 << 6U) | succ2;
    }
    if (J_UNLIKELY(sz == 3U)) { return code_point_replacement_character; }
    u32_t succ3 = c[3] - 0b10000000;
    if (J_UNLIKELY(succ3 > 0b00111111 || first > 0xF4 || (first == 0xF0 && succ1 < 0x10) || (first == 0xF4 && succ1 > 0xF))) {
      return code_point_replacement_character;
    }
    return ((first & 0b00000111U) << 18U) | (succ1 << 12U) | (succ2 << 6U) | succ3;
  }

  J_INLINE_GETTER u32_t utf8_get_code_point(const char * J_NOT_NULL const c) noexcept {
    return utf8_get_code_point((const u8_t *)c);
  }

  J_INLINE_GETTER u32_t utf8_get_code_point(const char * J_NOT_NULL const c,
                                            const char * J_NOT_NULL const end) noexcept {
    return utf8_get_code_point((const u8_t *)c, (const u8_t*)end);
  }

  J_INLINE_GETTER constexpr u32_t utf8_code_point_bytes(const u32_t codepoint) noexcept {
    if (codepoint < 0x00080U) { return 1U; }
    if (codepoint < 0x00800U) { return 2U; }
    if (codepoint < 0x10000U) { return 3U; }
    return 4U;
  }

  /// Encode codepoint into target, returning pointer to after the codepoint.
  ///
  /// Target must have space for the UTF-8 encoded codepoint available.
  inline J_RETURNS_NONNULL constexpr char * utf8_encode(
    char * J_NOT_NULL target,
    const u32_t codepoint) noexcept
  {
    if (codepoint < 0x00080U) {
      *target++ = codepoint;
    } else if (codepoint < 0x00800U) {
      *target++ = 0b11000000 | (codepoint >> 6U);
      *target++ = 0b10000000 | (codepoint & 0b00111111);
    } else if (codepoint < 0x10000U) {
      *target++ = 0b11100000 | (codepoint >> 12U);
      *target++ = 0b10000000 | ((codepoint >> 6) & 0b00111111);
      *target++ = 0b10000000 | (codepoint & 0b00111111);
    } else {
      *target++ = 0b11110000 | (codepoint >> 18U);
      *target++ = 0b10000000 | ((codepoint >> 12) & 0b00111111);
      *target++ = 0b10000000 | ((codepoint >> 6) & 0b00111111);
      *target++ = 0b10000000 | (codepoint & 0b00111111);
    }
    return target;
  }

  [[nodiscard]] inline u32_t utf8_num_successors(const char * J_NOT_NULL c) noexcept {
    if (!utf8_is_successor_byte(c[0])) {
      return 0;
    }
    if (!utf8_is_successor_byte(c[1])) {
      return 1;
    }
    if (!utf8_is_successor_byte(c[2])) {
      return 2;
    }
    return 3;
  }

  [[nodiscard]] inline J_RETURNS_NONNULL const char * utf8_synchronize_forward(
    const char * J_NOT_NULL c,
    const char * J_NOT_NULL limit
  ) {
    J_ASSUME(c < limit);
    if (c + 1U == limit || !utf8_is_successor_byte(c[0])) {
      return c;
    }
    if (c + 2U == limit || !utf8_is_successor_byte(c[1])) {
      return c + 1;
    }
    if (c + 3U == limit || !utf8_is_successor_byte(c[2])) {
      return c + 2;
    }
    return c + 3;
  }
}
