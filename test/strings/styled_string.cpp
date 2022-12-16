#include <detail/preamble.hpp>

#include "strings/styling/styled_string.hpp"

using namespace j::strings;

TEST_SUITE_BEGIN("Strings - Styled String");

namespace {
  template<sz_t I>
  void assert_styled_string(const styled_string & s,
                            const char *(&& strings)[I],
                            const style (&& styles)[I])
  {
    sz_t i = 0;
    for (auto & part : s) {
      REQUIRE(i < I);
      REQUIRE(part.style() == styles[i]);
      REQUIRE(part.string() == strings[i]);
      ++i;
    }
    REQUIRE(i == I);
  }
}

TEST_CASE("Empty string") {
  styled_string s;
  REQUIRE(s.begin() == s.end());
}

TEST_CASE("Styleless string") {
  styled_string s{"Hello", {}};
  assert_styled_string(s, {"Hello"}, {style()});
}

TEST_CASE("Completely styled string") {
  string_style st;
  st.push_back(string_style_segment(0, style(bold)));
  styled_string s{"Hello", st};
  assert_styled_string(s, {"Hello"}, {style(bold)});
}

TEST_CASE("Half styled string") {
  string_style st;
  st.push_back(string_style_segment(6, style(bold)));
  styled_string s{"Hello, world!", st};
  assert_styled_string(s, {"Hello,", " world!"}, {style(), style(bold)});
}

TEST_CASE("Distinctly styled string") {
  string_style st;
  st.push_back(string_style_segment(0, style(italic)));
  st.push_back(string_style_segment(6, style(bold)));
  styled_string s{"Hello, world!", st};
  assert_styled_string(s, {"Hello,", " world!"}, {style(italic), style(bold)});
}

TEST_SUITE_END();
