#include <detail/preamble.hpp>

#include "strings/string.hpp"
#include "strings/string_view.hpp"
#include "strings/string_hash.hpp"

TEST_SUITE("Strings - Equality") {

  using namespace j::strings;

  namespace {
    template<typename T, typename U>
    void assert_equals(T && a, U && b) {
      REQUIRE_UNARY(a == b);
      REQUIRE_UNARY(b == a);
      for (i32_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i] == b[i]);
      }
      REQUIRE(j::strings::hash_string(a) == j::strings::hash_string(b));
    }

    template<typename Fn>
    void with_all(Fn && fn) {
      static_cast<Fn &&>(fn)("");
      static_cast<Fn &&>(fn)("a");
      static_cast<Fn &&>(fn)("ab");
      static_cast<Fn &&>(fn)("abc");
      static_cast<Fn &&>(fn)("abcd");
      static_cast<Fn &&>(fn)("abcde");
      static_cast<Fn &&>(fn)("abcdef");
      static_cast<Fn &&>(fn)("abcdefg");
      static_cast<Fn &&>(fn)("abcdefgh");
      static_cast<Fn &&>(fn)("abcdefghi");
      static_cast<Fn &&>(fn)("abcdefghij");
    }
  }

  TEST_CASE("strings::string vs. const char array") {
    with_all([](const auto & str) {
      const string s(str);
      assert_equals(s, str);
    });
  }

  TEST_CASE("strings::string vs. const char *") {
    with_all([](const char * str) {
      const string s(str);
      assert_equals(s, str);
    });
  }

  TEST_CASE("strings::string vs. strings::string") {
    with_all([](string str) {
      assert_equals(str, str);
    });
  }
}
