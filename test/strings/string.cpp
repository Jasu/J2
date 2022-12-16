#include <detail/preamble.hpp>

#include "strings/string.hpp"
#include "mem/unique_ptr.hpp"

TEST_SUITE_BEGIN("Strings - String");

using namespace j::strings;
namespace m = j::mem;

namespace {
  const char zeroes[8] = {0};

  void assert_string_base(const string & s, const char * J_NOT_NULL cstr, u32_t len) {
    const char * d = s.data();
    REQUIRE_EQ(s.size(), len);
    REQUIRE_UNARY(::j::memcmp(d, cstr, len) == 0);
    d += len;
    const u32_t tail = (8 - reinterpret_cast<uptr_t>(d)) & 0x7U;
    REQUIRE_UNARY(!::j::memcmp(d, zeroes, tail));
    REQUIRE_UNARY(s == const_string_view(cstr, len));
  }

  void assert_string(const string & s, const char * J_NOT_NULL cstr, u32_t len) {
    assert_string_base(s, cstr, len);
    REQUIRE_EQ(s, string(cstr, len));
  }

  m::unique_ptr<const char[]> make_large_string() {
    auto result = m::unique_ptr<char[]>(::new char[0x8000]);
    for (u32_t i = 0; i < 0x7FFF; ++i) {
      result[i] = i;
    }
    return static_cast<m::unique_ptr<char[]> &&>(result);
  }

  void assert_string_construct(const char * J_NOT_NULL const str, i32_t sz) {
    string s{str, sz};
    assert_string_base((const string &)s, str, sz);
    assert_string_base(const_string_view(str, sz), str, sz);
    assert_string_base(string(static_cast<string &&>(s)), str, sz);
  }

  void test_erases(const_string_view src,
                   u32_t min_result_sz,
                   u32_t max_result_sz) {
    auto buf = m::unique_ptr<char[]>(::new char[src.size()]);
    for (u32_t erase_pos = 0, len = src.size(); erase_pos <= len; ++erase_pos) {
      if (erase_pos) {
        buf[erase_pos - 1] = src[erase_pos - 1];
      }
      for (u32_t erase_len = len - max_result_sz, max_erase_len = len - ::j::max(erase_pos, min_result_sz);
           erase_len <= max_erase_len;
           ++erase_len) {
        ::j::memcpy(buf.get() + erase_pos, src.data() + erase_pos + erase_len, len - erase_pos - erase_len);
        string s(src.data(), len);
        s.erase(erase_pos, erase_len);
        assert_string(s, buf.get(), len - erase_len);
      }
    }
  }
}

TEST_CASE("Small strings") {
  const char * small = "123456";
  for (u32_t i = 0; i < 7; ++i, ++small) {
    assert_string_construct(small, 6 - i);
  }
}

TEST_CASE("Medium strings") {
  auto large = make_large_string();
  for (u32_t i = 0; i < 0x8000 - 7; ++i) {
    assert_string_construct(large.get() + i, 0x7FFF - i);
  }
}

TEST_CASE("Small-string erase") {
  const char * str = "123456";
  for (u32_t i = 0; i < 7; ++i) {
    test_erases(str + i, 0, 6 - i);
  }
}

TEST_CASE("Medium string to small string erase") {
  auto large = make_large_string();
  test_erases({ large.get(), 0x7FFF }, 0, 6);
  test_erases("1234567", 0, 6);
}

TEST_SUITE_END();
