#include <detail/preamble.hpp>

#include "strings/unicode/utf8.hpp"

TEST_SUITE_BEGIN("Strings - UTF-8");

namespace s = j::strings;

TEST_CASE("Code point size, valid") {
  // Single-byte
  for (u8_t i = 0; i < 0x80; ++i) {
    REQUIRE(s::utf8_code_point_bytes(&i) == 1U);
  }

  // Two-byte
  for (u8_t i = 0xC2; i < 0xE0; ++i) {
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0x80}) == 2U);
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0xBF}) == 2U);
  }

  // Three-byte
  for (u8_t i = 0xE0; i < 0xF0; ++i) {
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0x80, 0x80}) == 3U);
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0xBF, 0xBF}) == 3U);
  }

  // Four-byte
  for (u8_t i = 0xF0; i < 0xF5; ++i) {
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0x80, 0x80, 0x80}) == 4U);
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0xBF, 0xBF, 0xBF}) == 4U);
  }
}

TEST_CASE("Code point size backwards, valid") {
  // Single-byte (identical case to above...
  for (u8_t i = 0; i < 0x80; ++i) {
    REQUIRE(s::utf8_code_point_bytes_backwards(&i) == 1U);
  }

  // Two-bytes
  for (u8_t i = 0xC2; i < 0xE0; ++i) {
    const u8_t chars1[] = {i, 0x80};
    REQUIRE(s::utf8_code_point_bytes_backwards(chars1 + 1) == 2U);
    const u8_t chars2[] = {i, 0xBF};
    REQUIRE(s::utf8_code_point_bytes_backwards(chars2 + 1) == 2U);
  }

  // Three-byte
  for (u8_t i = 0xE0; i < 0xF0; ++i) {
    const u8_t chars1[] = {i, 0x80, 0x80};
    const u8_t chars2[] = {i, 0xBF, 0xBF};
    REQUIRE(s::utf8_code_point_bytes_backwards(chars1 + 2) == 3U);
    REQUIRE(s::utf8_code_point_bytes_backwards(chars2 + 2) == 3U);
  }

  // Four-byte
  for (u8_t i = 0xF0; i < 0xF5; ++i) {
    const u8_t chars1[] = {i, 0x80, 0x80, 0x80};
    REQUIRE(s::utf8_code_point_bytes_backwards(chars1 + 3) == 4U);
    const u8_t chars2[] = {i, 0xBF, 0xBF, 0xBF};
    REQUIRE(s::utf8_code_point_bytes_backwards(chars2 + 3) == 4U);
  }
}

TEST_CASE("Code point size, invalid first bytes") {
  // Range 0x80..0xBF are continuation bytes and cannot start a code point.
  // Two-byte code points start at 0xC2, since the no code points can have their
  // shortest encoding below that.
  for (u8_t i = 0x80; i < 0xC2; ++i) {
    REQUIRE(s::utf8_code_point_bytes(&i) == 1U);
  }

  // Encodings starting with 0xF5 decode to code points above the Unicode
  // codespace (which will never be assigned, thanks tu UTF-16).
  // Encodings starting with 0xF8 are invalid UTF-8, since the first five
  // bytes would be one.
  for (u8_t i = 0xF5; i != 0; ++i) {
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){i, 0xBF, 0xBF, 0xBF}) == 1U);
  }
}

TEST_CASE("Code point size, invalid following bytes") {
  for (u32_t i = 0; i < 0xFF; ++i) {
    // Skip valid continuations
    if ((i & 0b11000000) == 0b10000000) {
      continue;
    }
    // Two-byte starter
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){0xC2, (u8_t)i}) == 1U);
    // Three-byte starter
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){0xE0, (u8_t)i, 0x80}) == 1U);
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){0xE0, 0x80, (u8_t)i}) == 2U);
    // Four-byte starter
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){0xF0, (u8_t)i, 0x80, 0x80}) == 1U);
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){0xF0, 0x80, (u8_t)i, 0x80}) == 2U);
    REQUIRE(s::utf8_code_point_bytes((u8_t[]){0xF0, 0x80, 0x80, (u8_t)i}) == 3U);
  }
}

TEST_CASE("Code point size, invalid following bytes, reversed") {
  for (u32_t i = 0; i < 0xFF; ++i) {
    // Skip valid continuations
    if ((i & 0b11000000) == 0b10000000) {
      continue;
    }
    // Two-byte starter
    u8_t two_byte_starter[]{0xC2, (u8_t)i};
    REQUIRE(s::utf8_code_point_bytes_backwards(two_byte_starter) == 1U);
    REQUIRE(s::utf8_code_point_bytes_backwards(two_byte_starter + 1) == 1U);

    // Three-byte starter
    u8_t three_byte_starters1[]{0xE0, (u8_t)i, 0x80};
    u8_t three_byte_starters2[]{0xE0, 0x80, (u8_t)i};
    REQUIRE(s::utf8_code_point_bytes_backwards(three_byte_starters1) == 1U);
    REQUIRE(s::utf8_code_point_bytes_backwards(three_byte_starters1 + 1) == 1U);
    REQUIRE(s::utf8_code_point_bytes_backwards(three_byte_starters2 + 1) == 2U);

    // Four-byte starter
    u8_t four_byte_starters1[]{0xF0, (u8_t)i, 0x80, 0x80};
    u8_t four_byte_starters2[]{0xF0, 0x80, (u8_t)i, 0x80};
    u8_t four_byte_starters3[]{0xF0, 0x80, 0x80, (u8_t)i};
    REQUIRE(s::utf8_code_point_bytes_backwards(four_byte_starters1) == 1U);
    REQUIRE(s::utf8_code_point_bytes_backwards(four_byte_starters1 + 1) == 1U);
    REQUIRE(s::utf8_code_point_bytes_backwards(four_byte_starters2 + 1) == 2U);
    REQUIRE(s::utf8_code_point_bytes_backwards(four_byte_starters2 + 2) == 1U);
    REQUIRE(s::utf8_code_point_bytes_backwards(four_byte_starters3 + 1) == 2U);
    REQUIRE(s::utf8_code_point_bytes_backwards(four_byte_starters3 + 2) == 3U);
  }
}

TEST_CASE("Is truncated, valid") {
  const u8_t one_byte[]{0x0F, 0x00, 0x00, 0x00};
  const u8_t two_byte[]{0xC2, 0x80, 0x00, 0x00};
  const u8_t three_byte[]{0xE0, 0x80, 0x80, 0x00};
  const u8_t four_byte[]{0xF0, 0x80, 0x80, 0x80};

  REQUIRE(s::utf8_code_point_truncated_by(one_byte) == 0U);
  REQUIRE(s::utf8_code_point_truncated_by(two_byte) == 0U);
  REQUIRE(s::utf8_code_point_truncated_by(three_byte) == 0U);
  REQUIRE(s::utf8_code_point_truncated_by(four_byte) == 0U);

  REQUIRE(s::utf8_code_point_truncated_by_backwards(one_byte) == 0U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(two_byte + 1) == 0U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(three_byte + 2) == 0U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(four_byte + 3) == 0U);
}

TEST_CASE("Is truncated, invalid") {
  const u8_t two_byte0[]{0xC2, 0x70, 0x00, 0x00};
  const u8_t three_byte0[]{0xE0, 0x70, 0x70, 0x00};
  const u8_t three_byte1[]{0xE0, 0x80, 0x70, 0x00};
  const u8_t four_byte0[]{0xF0, 0x70, 0x70, 0x70};
  const u8_t four_byte1[]{0xF0, 0x80, 0x70, 0x70};
  const u8_t four_byte2[]{0xF0, 0x80, 0x80, 0x70};

  REQUIRE(s::utf8_code_point_truncated_by(two_byte0) == 1U);
  REQUIRE(s::utf8_code_point_truncated_by(three_byte0) == 2U);
  REQUIRE(s::utf8_code_point_truncated_by(three_byte1) == 1U);
  REQUIRE(s::utf8_code_point_truncated_by(four_byte0) == 3U);
  REQUIRE(s::utf8_code_point_truncated_by(four_byte1) == 2U);
  REQUIRE(s::utf8_code_point_truncated_by(four_byte2) == 1U);

  REQUIRE(s::utf8_code_point_truncated_by_backwards(two_byte0) == 1U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(three_byte1) == 2U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(three_byte1 + 1) == 1U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(four_byte0) == 3U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(four_byte1 + 1) == 2U);
  REQUIRE(s::utf8_code_point_truncated_by_backwards(four_byte2 + 2) == 1U);
}

TEST_CASE("Num successors") {
  u8_t buf[4096];
  ::j::memset(buf, 0x80, 4096U);
  for (u32_t i = 1; i <= 4096; ++i) {
    REQUIRE(s::utf8_num_leading_successors(buf, i) == i);
    REQUIRE(s::utf8_num_trailing_successors(buf + 4095, i) == i);
  }
  for (u32_t i = 1; i <= 4095; ++i) {
    buf[i] = 0;
    REQUIRE(s::utf8_num_leading_successors(buf, i) == i);
    buf[i] = 0x80;
    for (u8_t c : (const u8_t[]){ 0, 0xC2, 0xE0, 0xF0 }) {
      buf[4095 - i] = c;
      REQUIRE(s::utf8_num_trailing_successors(buf + 4095, i) == i);
    }
    buf[4095 - i] = 0x80;
  }

  for (u32_t i = 1; i <= 64; ++i) {
    for (u32_t j = 0; j < i; ++j) {
      buf[j] = 0;
      REQUIRE(s::utf8_num_leading_successors(buf, i) == j);
      buf[j] = 0x80;
      buf[4095 - j] = 0;
      REQUIRE(s::utf8_num_trailing_successors(buf + 4095, i) == j);
      buf[4095 - j] = 0xC2;
      REQUIRE(s::utf8_num_trailing_successors(buf + 4095, i) == j - 1);
      buf[4095 - j] = 0xE0;
      REQUIRE(s::utf8_num_trailing_successors(buf + 4095, i) == j - 2);
      buf[4095 - j] = 0xF0;
      REQUIRE(s::utf8_num_trailing_successors(buf + 4095, i) == j - 3);
      buf[4095 - j] = 0x80;
    }
  }
}

TEST_SUITE_END();
