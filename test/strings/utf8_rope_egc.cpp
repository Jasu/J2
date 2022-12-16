#include "egc.hpp"

TEST_SUITE_BEGIN("Strings - UTF-8 Rope Grapheme Clusters");

namespace s = j::strings;

TEST_CASE("Empty") {
  s::utf8_rope r;
  REQUIRE_UNARY(r.grapheme_clusters().begin() == r.grapheme_clusters().end());
}

TEST_CASE("Single char ASCII") {
  s::utf8_rope r;
  r += "A";
  assert_grapheme_clusters(r, {"A"});
}

TEST_CASE("Multiple chars ASCII") {
  s::utf8_rope r;
  SUBCASE("Single chunk") {r += "Hello"; }
  SUBCASE("Two chunks") { r += "Hel"; r += "lo"; }
  SUBCASE("Parts") { r += "H"; r += "e"; r += "l"; r += "l"; r += "o"; }
  assert_grapheme_clusters(r, {"H", "e", "l", "l", "o"});
}

TEST_CASE("CRLF Single") {
  s::utf8_rope r;
  SUBCASE("Single chunk") { r += "\r\n"; }
  SUBCASE("Split chunk") { r += "\r"; r += "\n"; }
  assert_grapheme_clusters(r, {"\r\n"});
}
TEST_CASE("CRLF") {
  const char * str = "A\r\nB\r\rC\n\rD\r\n\r\nE";
  const u32_t len = ::j::strlen(str);
  for (u32_t i = 0; i <= len; ++i) {
    for (u32_t j = 0; j <= len - i; ++j) {
      for (u32_t k = 0; k <= len - i - j; ++k) {
        s::utf8_rope r;
        if (i) { r.append(str, i); }
        u32_t offset = i;
        if (j) { r.append(str + offset, j); }
        offset += j;
        if (k) { r.append(str + offset, k); }
        offset += k;
        if (offset < len) {
          r.append(str + offset, len - offset);
        }
        assert_grapheme_clusters(r, {"A", "\r\n", "B", "\r", "\r", "C", "\n", "\r", "D", "\r\n", "\r\n", "E"});
      }
    }
  }
}

TEST_SUITE_END();
