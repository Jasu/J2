#include <detail/preamble.hpp>

#include "hzd/crc32.hpp"

TEST_SUITE_BEGIN("Hzd - CRC32");

TEST_CASE("Equivalence") {
  REQUIRE(j::crc32(0, (u16_t)0xAB12) == j::crc32(j::crc32(0, (u8_t)0x12), (u8_t)0xAB));
  REQUIRE(j::crc32(0, (u32_t)0x9221AB12) == j::crc32(j::crc32(j::crc32(j::crc32(0, (u8_t)0x12), (u8_t)0xAB), (u8_t)0x21), (u8_t)0x92));
  REQUIRE(j::crc32(0, (u32_t)0x9221AB12) == j::crc32(j::crc32(0, (u16_t)0xAB12), (u16_t)0x9221));
  REQUIRE(j::crc32(0, (u64_t)0xF90510549221AB12ULL) == j::crc32(j::crc32(0, (u32_t)0x9221AB12), (u32_t)0xF9051054));
}

TEST_CASE("Buffer equivalence") {
  const char * const str = "Hello, World, and Goodbye.";
  u32_t crc = 0U;
  for (u32_t i = 0U; i < ::j::strlen(str); ++i) {
    crc = j::crc32(crc, str[i]);
    REQUIRE(crc == j::crc32(0, str, i + 1U));
  }
}

TEST_CASE("Combine") {
  const char * const str = "Hello, World!";
  for (u32_t i = 0U; i <= ::j::strlen(str); ++i) {

    REQUIRE(j::crc32(0, str, ::j::strlen(str))
            == j::crc32_combine(j::crc32(0, str, i), j::crc32(0, str + i, ::j::strlen(str) - i), ::j::strlen(str) - i));
  }

}

TEST_SUITE_END();
