#include <detail/preamble.hpp>

#include "strings/unicode/rope/rope_utf8.hpp"
#include "strings/unicode/rope/landmark_array_impl.hpp"
#include "strings/unicode/utf8_string_view_access.hpp"

TEST_SUITE_BEGIN("Strings - UTF-8 Rope 2");

namespace s = j::strings;

namespace {
  void assert_utf8_rope(const s::rope_utf8 & r) {
    s::line_break_state lbr{};
    u64_t total_lines = 0U;
    u64_t total_size = 0U;
    for (auto & c : r.chunks()) {
      const auto metrics_before = c.ref.node()->cumulative_metrics(c.ref.index());
      REQUIRE_EQ(metrics_before.num_hard_breaks, total_lines);
      REQUIRE_EQ(metrics_before.size_bytes, total_size);
      u32_t num_lines = 0U;
      total_size += c.text.size();

      auto & lmks = c.landmarks;
      auto begin = c.text.code_points().begin(),
           it = begin,
           end = c.text.code_points().end();
      for (auto & lmk : lmks) {
        u32_t lmk_lines = 0U;
        for (const char * end = it.ptr() + lmk.offset(); it < end; ++it) {
          if (lbr.get_break_type(*it) == s::line_break_type::mandatory) {
            ++lmk_lines;
          }
          lbr.update_state(*it);
          REQUIRE_UNARY(it < end);
        }
        REQUIRE_EQ(lmk.lbr_state_after(), lbr);
        REQUIRE_EQ(lmk.hard_breaks_before(), lmk_lines);
        num_lines += lmk_lines;
      }
      REQUIRE_UNARY(it == end);
      total_lines += num_lines;

      REQUIRE_UNARY(c.landmarks.back().lbr_state_after() == lbr);
      REQUIRE_EQ(c.metrics().num_hard_breaks, num_lines);
      const auto node_delta = c.ref.node()->metrics_delta(c.ref.index());
      REQUIRE_UNARY(c.metrics() == node_delta);
      REQUIRE_UNARY(c.metrics().num_hard_breaks == node_delta.num_hard_breaks);
    }
  }

  void assert_utf8_rope_contents(const s::rope_utf8 & r, const s::const_string_view * chunks, u32_t sz, bool exact = false) {
    REQUIRE_UNARY(!r.empty());
    u32_t i = 0U;
    auto it = r.bytes().begin();
    auto it_codes = r.code_points().begin();
    const auto end = r.bytes().end();
    const auto end_codes = r.code_points().end();
    u32_t bytes = 0U;
    for (auto c : r.chunks()) {
      auto t = c.text;
      REQUIRE_UNARY(t);
      while (t) {
        REQUIRE_LT(i, sz);
        auto expected = t.take_prefix(chunks[i].size());
        if (exact) {
          REQUIRE_UNARY(!t);
        }
        bytes += chunks[i].size();
        REQUIRE(expected == chunks[i]);
        for (auto code : s::const_utf8_string_view(chunks[i]).code_points()) {
          REQUIRE_UNARY(it_codes != end_codes && code == *it_codes);
          ++it_codes;
        }
        for (auto ch : chunks[i]) {
          REQUIRE_UNARY(it != end);
          REQUIRE_UNARY(ch == *it);
          ++it;
        }
        ++i;
      }
    }
    REQUIRE_UNARY(it == end);
    REQUIRE_UNARY(it_codes == end_codes);
    REQUIRE_EQ(i, sz);
    REQUIRE_EQ(r.size_bytes(), bytes);
    assert_utf8_rope(r);
  }

  void assert_utf8_rope_contents_exact(const s::rope_utf8 & r, const s::const_string_view * chunks, u32_t sz) {
    assert_utf8_rope_contents(r, chunks, sz, true);
  }

  template<u32_t I>
  void assert_utf8_rope_contents(const s::rope_utf8 & r, const s::const_string_view (& chunks)[I]) {
    assert_utf8_rope_contents(r, chunks, I);
  }

  template<u32_t I>
  void assert_utf8_rope_contents_exact(const s::rope_utf8 & r, const s::const_string_view (& chunks)[I]) {
    assert_utf8_rope_contents_exact(r, chunks, I);
  }

  template<typename Fn, u32_t N>
  void test_erase_cases(Fn && build_rope, u64_t index, u64_t length, const s::const_string_view (& chunks)[N]) {
    {
      s::rope_utf8 r;
      static_cast<Fn &&>(build_rope)(r);
      r.erase(index, length);
      assert_utf8_rope_contents(r, chunks, N);
    }

    {
      s::rope_utf8 r;
      static_cast<Fn &&>(build_rope)(r);
      auto it = r.erase(r.bytes().iterate_at(index), length);
      assert_utf8_rope_contents(r, chunks, N);
      REQUIRE_UNARY(it == r.bytes().iterate_at(index));
    }

    {
      s::rope_utf8 r;
      static_cast<Fn &&>(build_rope)(r);
      auto it = r.erase(r.bytes().iterate_at(index), r.bytes().iterate_at(index + length));
      assert_utf8_rope_contents(r, chunks, N);
      REQUIRE_UNARY(it == r.bytes().iterate_at(index));
    }
  }
}

TEST_CASE("Empty") {
  s::rope_utf8 r;
  REQUIRE_UNARY(r.empty());
  REQUIRE_UNARY(r.chunks().begin() == r.chunks().end());
  REQUIRE_UNARY(r.bytes().begin() == r.bytes().end());
}

TEST_CASE("Simple") {
  s::const_string_view strs[]{"Hello", "darkness", "my", "old", "friend"};
  s::rope_utf8 r;
  u64_t sz = 0U, n = 0U;
  for (auto str : strs) {
    r.insert(sz, str);
    sz += str.size();
    assert_utf8_rope_contents(r, strs, ++n);
  }
}

TEST_CASE("Insert split") {
  s::rope_utf8 r;
  r.insert(0U, "HelloWorld");
  r.insert(5U, ", ");
  assert_utf8_rope_contents(r, {"Hello", ", ", "World"});
}

TEST_CASE("Insert merge cases") {
  const s::const_string_view truncated[]{
    "A",
    "\xC2",
    "\xE0",
    "\xF0",
  };
  u8_t truncated_by = 0U;
  s::const_string_view left_prefix, right_suffix;
  SUBCASE("Prefix, suffix") { left_prefix = "Hello"; right_suffix = "World"; }
  SUBCASE("Prefix") { left_prefix = "Hello"; }
  SUBCASE("Suffix") { right_suffix = "World"; }
  SUBCASE("None") { }
  for (auto s : truncated) {
    for (u8_t num_successors = 0U; num_successors <= 4U; ++num_successors) {
      for (u8_t successors_left = 0U; successors_left <= num_successors; ++successors_left) {
        s::string left = left_prefix + s, right;
        for (u8_t i = 0; i < num_successors; ++i) {
          (i < successors_left ? left : right) += "\x80";
        }

        const bool is_left_valid = successors_left >= truncated_by;

        s::string expected_left, expected_right;
        if (is_left_valid) {
          expected_left = left;
          expected_right = right;
        } else {
          expected_left = left_prefix + s;
          // When prepending, the left part will steal from the right part to make it valid.
          for (u8_t i = 0; i < num_successors; ++i) {
            if (i < truncated_by) {
              expected_left += "\x80";
            } else {
              expected_right += "\x80";
            }
          }
        }
        expected_right += right_suffix;
        right += right_suffix;

        {
          s::rope_utf8 r;
          r.append(right);
          r.prepend(left);
          if (expected_left && expected_right) {
            assert_utf8_rope_contents(r, {expected_left, expected_right});
          } else if (expected_left) {
            assert_utf8_rope_contents(r, {expected_left});
          } else {
            assert_utf8_rope_contents(r, {expected_right});
          }
        }

        // When a string starting with successors is appended to a truncated string,
        // the truncated part will be stolen to the string (even though it won't necessarily
        // become valid, e.g. "Asdf\xE0" + "\x80Hello" will become "Asdf" + "\xE0\x80Hello".
        if (!is_left_valid && num_successors - successors_left > 0) {
          expected_left = left_prefix;
          expected_right = s;
          for (u8_t i = 0; i < num_successors; ++i) {
            expected_right += "\x80";
          }
          expected_right += right_suffix;
        }

        {
          s::rope_utf8 r;
          r.append(left);
          r.append(right);
          if (expected_left && expected_right) {
            assert_utf8_rope_contents(r, {expected_left, expected_right});
          } else if (expected_left) {
            assert_utf8_rope_contents(r, {expected_left});
          } else {
            assert_utf8_rope_contents(r, {expected_right});
          }
        }
      }
    }
    ++truncated_by;
  }
}

TEST_CASE("Range erase cases") {
  test_erase_cases(
    [](s::rope_utf8 & r) { r.append("Hello\xC2, \x80World!"); },
    6, 2,
    {"Hello", "\xC2\x80", "World!"}
  );

  test_erase_cases(
    [](s::rope_utf8 & r) {
      r.append("Hello\xC2");
      r.append(", ");
      r.append("\x80World!");
    },
    6, 2,
    {"Hello", "\xC2\x80", "World!"}
  );
  test_erase_cases(
    [](s::rope_utf8 & r) {
      r.append("\xC2 \x80");
    },
    1, 1,
    {"\xC2\x80"}
  );

  test_erase_cases(
    [](s::rope_utf8 & r) {
      r.append("\xC2\xC2\x80");
    },
    1, 1,
    {"\xC2\x80"}
  );

  test_erase_cases(
    [](s::rope_utf8 & r) {
      r.append("\xC2\x80\x80");
    },
    1, 1,
    {"\xC2\x80"}
  );
}

TEST_CASE("Line breaks") {
  for (u32_t sequence = 0; sequence < 0b10000; ++sequence) {
    s::rope_utf8 r;
    for (u32_t i = 0U; i < 50U; ++i) {
      const char * str = nullptr;
      switch (i % 4) {
      case 0: str = "Line\nBreak"; break;
      case 1: str = "\r\nLine"; break;
      case 2: str = "LineB\nreak"; break;
      case 3: str = "LineBrk\rOnly"; break;
      }
      if (sequence & (1 << (i % 4))) {
        r.prepend(str);
      } else {
        r.append(str);
      }
      assert_utf8_rope(r);
      REQUIRE(r.num_hard_breaks() == i + 1U);
    }
  }
}

TEST_CASE("CRLF") {
  s::rope_utf8 r;
  r.append("\r\n\r\n\r\n\r\n\r\nHello");
  REQUIRE(r.num_hard_breaks() == 5U);
  assert_utf8_rope(r);
  r.erase(0, 1);
  REQUIRE(r.num_hard_breaks() == 5U);
  assert_utf8_rope(r);
  r.erase(0, 1);
  REQUIRE(r.num_hard_breaks() == 4U);
  assert_utf8_rope(r);
  r.erase(2, 2);
  REQUIRE(r.num_hard_breaks() == 3U);
  assert_utf8_rope(r);
  r.erase(1, 1);
  REQUIRE(r.num_hard_breaks() == 3U);
  assert_utf8_rope(r);
  r.erase(1, 1);
  REQUIRE(r.num_hard_breaks() == 2U);
  assert_utf8_rope(r);
}

TEST_CASE("CRLF 2") {
  s::rope_utf8 r;
  r.append("Hello\r");
  assert_utf8_rope(r);
  r.append("World");
  assert_utf8_rope(r);
  r.append("\n.");
  assert_utf8_rope(r);
  REQUIRE(r.num_hard_breaks() == 2U);
  r.erase(6, 5);
  REQUIRE(r.num_hard_breaks() == 1U);
  assert_utf8_rope_contents(r, {"Hello\r", "\n."});
}

TEST_CASE("CRLF 3") {
  s::rope_utf8 r;
  r.append("A\r\nB");
  REQUIRE(r.num_hard_breaks() == 1U);
  assert_utf8_rope(r);
  r.insert(2, "C");
  REQUIRE(r.num_hard_breaks() == 2U);
  assert_utf8_rope_contents(r, {"A\r", "C", "\nB"});
}

TEST_CASE("Find line break simple") {
  s::rope_utf8 r;
  r.append("Hello\nWorld\n.");
  REQUIRE(r.num_hard_breaks() == 2U);
  REQUIRE(r.find_line(0U) == r.bytes().begin());
  REQUIRE(r.find_line(1U) == r.bytes().iterate_at(6));
  REQUIRE(r.find_line(2U) == r.bytes().iterate_at(12));
}

TEST_CASE("Find line break complex") {
  s::rope_utf8 r;
  for (u8_t i = 0; i < 20; ++i) {
    r.append("HelloWorld");
  }
  for (u8_t i = 0; i < 20 * 10; i += 10) {
    r.insert(i, "\n");
    assert_utf8_rope(r);
  }
  REQUIRE(r.num_hard_breaks() == 20);
  REQUIRE(r.find_line(0).position() == 0U);
  for (u8_t i = 0U; i < 20U; ++i) {
    REQUIRE(r.find_line(i + 1U).position() == i * 10U + 1U);
  }
}

TEST_CASE("Find line break long") {
  char buf[60];
  for (u32_t i = 0; i < 60; ++i) {
    for (u32_t j = 0; j < 60; ++j) {
      buf[j] = i == j ? '\n' : 'a';
    }
    s::rope_utf8 r({buf, 60});
    REQUIRE(r.num_hard_breaks() == (i == 59 ? 0 : 1));
    if (i < 59) {
      REQUIRE(r.find_line(1).position() == i + 1);
    }
  }
}

TEST_CASE("Append in place simple") {
  s::rope_utf8 r;
  r.append("Hello");
  assert_utf8_rope_contents_exact(r, {"Hello"});
  r.append("Darkness"); // 13
  assert_utf8_rope_contents_exact(r, {"HelloDarkness"});
  r.append("My"); // 15
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMy"});
  r.append("Old"); // 18
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOld"});
  r.append("Friend"); // 24
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriend"});
  r.append("I've"); // 28
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've"});
  r.append("Come"); // 32
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've", "Come"});
  r.append("To");
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've", "ComeTo"});
  r.append("Talk");
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've", "ComeToTalk"});
  r.append("With");
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've", "ComeToTalkWith"});
  r.append("You");
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've", "ComeToTalkWithYou"});
  r.append("Again");
  assert_utf8_rope_contents_exact(r, {"HelloDarknessMyOldFriendI've", "ComeToTalkWithYouAgain"});

  r = s::rope_utf8();
  char buf[30];
  for (i32_t i = 0; i < 29; ++i) {
    char c = 'A' + i;
    buf[i] = c;
    r.append({&c, 1});
    assert_utf8_rope_contents_exact(r, {{buf, i + 1}});
  }
  r.append("x");
  assert_utf8_rope_contents_exact(r, {{buf, 29}, "x"});
}

TEST_CASE("Prepend in place simple") {
  s::rope_utf8 r;
  r.append("123456789012345678901234567890");
  r.erase(0, 29);
  assert_utf8_rope_contents_exact(r, {"0"});

  char buf[30];
  buf[29] = '0';
  for (i32_t i = 0; i < 29; ++i) {
    char c = 'A' + i;
    buf[28 - i] = c;
    r.prepend({&c, 1});
    assert_utf8_rope_contents_exact(r, {{buf + 28 - i, i + 2}});
  }
  r.prepend("x");
  assert_utf8_rope_contents_exact(r, {"x", {buf, 30}});
}

TEST_SUITE_END();
