#include "lbr.hpp"
#include "strings/unicode/utf8_string_view.hpp"

TEST_SUITE_BEGIN("Strings - UTF-8 Line-breaking (edit)");

namespace s = j::strings;

namespace {
  struct J_TYPE_HIDDEN state {
    s::string m_string;
    s::utf8_rope m_rope;

    state & append(s::const_string_view str) {
      m_string += str;
      m_rope += str;
      return *this;
    }

    state & prepend(s::const_string_view str) {
      m_string.insert(0, str);
      m_rope.insert(0, str);
      return *this;
    }

    state & insert(u32_t at, s::const_string_view str) {
      m_string.insert(at, str);
      m_rope.insert(at, str);
      return *this;
    }

    state & erase(u32_t at, u32_t length) {
      m_string.erase(at, length);
      m_rope.erase(at, length);
      return *this;
    }

    u32_t size() const {
      return m_string.size();
    }

    void check() const {
      J_ASSERT_UTF8_ROPE(m_rope);
      CAPTURE(m_string);
      try {
        const u32_t sz = m_string.size();
        REQUIRE(sz == m_rope.size_bytes());
        s::line_break_state ls{};
        u32_t hard_breaks = 0U;
        for (auto it = m_rope.begin(), end = m_rope.end(); it != end; ++it) {
          const u32_t pos = it.byte_position();
          if (ls.get_break_type(*it) == s::line_break_type::mandatory) {
            ++hard_breaks;
            REQUIRE(m_rope.line_begin_index(hard_breaks) == pos);
            REQUIRE(m_rope.line_begin_iterator(hard_breaks) == it);
          }
          CAPTURE(pos);
          REQUIRE(hard_breaks == m_rope.num_lines_before(pos));
          ls.update_state(*it);
          REQUIRE(ls == m_rope.lbr_state_after(pos));
        }
        REQUIRE(m_rope.line_begin_iterator(0) == m_rope.begin());
        REQUIRE(m_rope.line_begin_iterator(hard_breaks + 1) == m_rope.end());
        REQUIRE(hard_breaks + 1U == m_rope.num_lines());
      } catch (...) {
        J_DUMP_UTF8_ROPE(m_rope);
        throw;
      }
    }
  };
}

TEST_CASE("ASCII no breaks") {
  state s;
  s.append("Hello").append("World").check();
}

TEST_CASE("ASCII CR LF") {
  s::const_string_view str;
  SUBCASE("Plain") { str = "Hello, World!"; }
  SUBCASE("LF End") { str = "Hello, World!\n"; }
  SUBCASE("LF Start") { str = "\nHello, World!"; }
  SUBCASE("LF Both") { str = "\nHello, World!\n"; }
  SUBCASE("LF Mid") { str = "Hello,\nWorld!"; }
  SUBCASE("LF All") { str = "\nHello,\nWorld!\n"; }
  SUBCASE("LF Interleave") { str = "\nH\nel\nl\no,\nW\no\nr\nl\nd\n!\n"; }
  for (u32_t i = 0U, sz = str.size(); i <= sz; ++i) {
    state s;
    s.append(str).check();
    s.insert(i, "\r").check();
  }
}

TEST_CASE("ASCII CR LF Erase") {
  s::const_string_view str = "Abc\r\n\r\n\n\r\r\n\nHello";
  for (u32_t i = 0U, sz = str.size(); i < sz; ++i) {
    for (u32_t j = 1U; j <= str.size() - i; ++j) {
      state s;
      s.append(str).erase(i, j).check();
    }
  }
}

TEST_SUITE_END();
