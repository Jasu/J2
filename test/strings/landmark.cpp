#include <detail/preamble.hpp>

#include "strings/unicode/rope/rope_utf8.hpp"
#include "strings/unicode/utf8_string_view_access.hpp"

TEST_SUITE_BEGIN("Strings - Landmark");

using namespace j::strings;

namespace {
  const char * const g_ascii_string = "Hello darkness, my old friend / I've come to talk with you again.";
  const char * const g_lbr_string = "A\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nBA\r\nB";
  void assert_lmk_array(const landmark_array & a, const_string_view sv, line_break_state lbr = {}) {
    i32_t num = ::j::clamp(1U, sv.size() / 8U, 4U);
    REQUIRE_EQ(a.size(), num);
    for (i32_t i = 0; i < num; ++i) {
      const_string_view prefix = sv.take_prefix(a.landmarks()[i].offset());
      u32_t num_breaks = 0;
      for (auto c : prefix) {
        if (lbr.get_break_type(c) == line_break_type::mandatory) {
          ++num_breaks;
        }
        lbr.update_state(c);
      }
      REQUIRE_EQ(a.landmarks()[i].hard_breaks_before(), num_breaks);
      REQUIRE_EQ(a.landmarks()[i].lbr_state_after(), lbr);
    }
    REQUIRE_UNARY(!sv);
  }

  line_break_state compute_lbr_state(const_utf8_string_view sv) {
    line_break_state s;
    for (i32_t c : sv.code_points()) {
      s.update_state(c);
    }
    return s;
  }

  i32_t count_hard_breaks(const_utf8_string_view sv, line_break_state s = {}) {
    i32_t result = 0;
    for (i32_t c : sv.code_points()) {
      if (s.get_break_type(c) == line_break_type::mandatory) {
        ++result;
      }
      s.update_state(c);
    }
    return result;
  }

  landmark_array make_landmark_array(const_string_view sv) {
    landmark_change chg{};
    landmark_array a;
    a.initialize(chg, sv);
    REQUIRE_EQ(chg.length_delta, sv.size());
    REQUIRE_EQ(chg.hard_breaks_delta, a.num_hard_breaks());
    REQUIRE_UNARY(chg.should_propagate());
    return a;
  }

  landmark_change assert_remove_prefix(landmark_array & arr, const_string_view sv, u16_t split_index) {
    line_break_state prev = compute_lbr_state(sv);
    i32_t old_breaks = count_hard_breaks(sv);
    const_string_view prefix = sv.take_prefix(split_index);
    landmark_change chg;
    arr.remove_prefix(chg, prefix, sv);
    REQUIRE_EQ(chg.length_delta, -(i64_t)prefix.size());
    REQUIRE_EQ(chg.hard_breaks_delta, count_hard_breaks(sv) - old_breaks);
    REQUIRE_EQ(chg.previous_lbr_state, prev);
    REQUIRE_EQ(chg.new_lbr_state, compute_lbr_state(sv));
    assert_lmk_array(arr, sv);
    landmark_array arr2(make_landmark_array(sv));
    REQUIRE_UNARY(arr == arr2);
    return chg;
  }

  landmark_change assert_remove_suffix(landmark_array & arr, const_string_view sv, u16_t split_index) {
    line_break_state prev = compute_lbr_state(sv);
    i32_t old_breaks = count_hard_breaks(sv);
    const_string_view suffix = sv.take_suffix(sv.size() - split_index);
    landmark_change chg;
    arr.remove_suffix(chg, sv, suffix);
    REQUIRE_EQ(chg.length_delta, -(i64_t)suffix.size());
    REQUIRE_EQ(chg.hard_breaks_delta, count_hard_breaks(sv) - old_breaks);
    REQUIRE_EQ(chg.previous_lbr_state, prev);
    REQUIRE_EQ(chg.new_lbr_state, compute_lbr_state(sv));
    assert_lmk_array(arr, sv);
    landmark_array arr2(make_landmark_array(sv));
    REQUIRE_UNARY(arr == arr2);
    return chg;
  }
}

TEST_CASE("Array dist simple") {
  for (i32_t i = 1; i < 16; ++i) {
    landmark_array a(make_landmark_array({g_ascii_string, i}));
    REQUIRE_EQ(a.size(), 1);
    assert_lmk_array(a, {g_ascii_string, i});
  }
  for (i32_t i = 16; i < 24; ++i) {
    landmark_array a(make_landmark_array({g_ascii_string, i}));
    REQUIRE_EQ(a.size(), 2);
    REQUIRE_EQ(a.landmarks()[0].offset(), 8);
    assert_lmk_array(a, {g_ascii_string, i});
  }
  for (i32_t i = 24; i < 32; ++i) {
    landmark_array a(make_landmark_array({g_ascii_string, i}));
    REQUIRE_EQ(a.size(), 3);
    REQUIRE_EQ(a.landmarks()[0].offset(), 8);
    REQUIRE_EQ(a.landmarks()[1].offset(), 8);
    assert_lmk_array(a, {g_ascii_string, i});
  }
  for (i32_t i = 32; i <= (i32_t)::j::strlen(g_ascii_string); ++i) {
    landmark_array a(make_landmark_array({g_ascii_string, i}));
    REQUIRE_EQ(a.size(), 4);
    REQUIRE_EQ(a.landmarks()[0].offset(), i / 4);
    REQUIRE_EQ(a.landmarks()[1].offset(), i / 4);
    REQUIRE_EQ(a.landmarks()[2].offset(), i / 4);
    assert_lmk_array(a, {g_ascii_string, i});
  }
}

TEST_CASE("Array dist simple line breaks") {
  char buf[40];
  ::j::memcpy(buf, g_ascii_string, 40);
  for (i32_t i = 1; i < 40; ++i) {
    for (i32_t j = 0; j < i; ++j) {
      buf[j] = '\n';
      landmark_array a(make_landmark_array({buf, i}));
      assert_lmk_array(a, {buf, i});
      buf[j] = g_ascii_string[j];
    }
  }
}

TEST_CASE("Array dist UTF-8") {
  char buf[40];
  ::j::memcpy(buf, g_ascii_string, 40);
  for (i32_t num = 1; num <= 8; ++num) {
    if (num <= 4) {
      buf[7 + num] = '\x80';
    } else {
      buf[11 + num] = '\x80';
    }
    for (i32_t i = 16; i < 32; ++i) {
      landmark_array a(make_landmark_array({buf, i}));
      REQUIRE_EQ(a.landmarks()[0].offset(), 8 + ::j::min(num, 3));
      if (i >= 24) {
        REQUIRE_EQ(a.landmarks()[1].offset(), 8 - ::j::min(num, 3) + ::j::clamp(0, num - 4, 3));
      }
      assert_lmk_array(a, {buf, i});
    }
  }
}

TEST_CASE("Array dist UTF-8 long") {
  char buf[64];
  ::j::memcpy(buf, g_ascii_string, 64);
  for (i32_t i = 32; i <= 64; ++i) {
    u32_t offset = i / 4;
    for (u32_t a = 0; a <= 4; ++a) {
      for (u32_t j = 0; j <= 4; ++j) {
        buf[offset + j] = j < a ? '\x80' : 'A';
      }
      for (u32_t b = 0; b <= 4; ++b) {
        for (u32_t j = 0; j <= 4; ++j) {
          buf[offset * 2 + j] = j < b ? '\x80' : 'A';
        }
        for (u32_t c = 0; c <= 4; ++c) {
          for (u32_t j = 0; j <= 4; ++j) {
            buf[offset * 3 + j] = j < c ? '\x80' : 'A';
          }
          landmark_array ar(make_landmark_array({buf, i}));
          REQUIRE_EQ(ar.size(), 4);
          REQUIRE_EQ(ar.landmarks()[0].offset(), offset + ::j::min(3, a));
          REQUIRE_EQ(ar.landmarks()[1].offset(), offset + ::j::min(3, b) - ::j::min(3, a));
          REQUIRE_EQ(ar.landmarks()[2].offset(), offset + ::j::min(3, c) - ::j::min(3, b));
          assert_lmk_array(ar, {buf, i});
        }
      }
    }
  }
}

TEST_CASE("Remove prefix") {
  for (i32_t i = 2; i <= (i32_t)::j::strlen(g_ascii_string); ++i) {
    for (i32_t j = 1; j < i; ++j) {
      landmark_array ar(make_landmark_array({g_ascii_string, i}));
      assert_remove_prefix(ar, {g_ascii_string, i }, j);
    }
  }
}

TEST_CASE("Remove suffix") {
  for (i32_t i = 2; i <= (i32_t)::j::strlen(g_ascii_string); ++i) {
    for (i32_t j = 1; j < i; ++j) {
      landmark_array ar(make_landmark_array({g_ascii_string, i}));
      assert_remove_suffix(ar, {g_ascii_string, i}, j);
    }
  }
}

TEST_CASE("Remove prefix (line breaks)") {
  for (i32_t i = 2; i <= (i32_t)::j::strlen(g_lbr_string); ++i) {
    for (i32_t j = 1; j < i; ++j) {
      landmark_array ar(make_landmark_array({g_lbr_string, i}));
      assert_remove_prefix(ar, {g_lbr_string, i}, j);
    }
  }
}

TEST_CASE("Remove suffix (line breaks)") {
  for (i32_t i = 2; i <= (i32_t)::j::strlen(g_lbr_string); ++i) {
    for (i32_t j = 1; j < i; ++j) {
      landmark_array ar(make_landmark_array({g_lbr_string, i}));
      assert_remove_suffix(ar, {g_lbr_string, i}, j);
    }
  }
}

TEST_CASE("Remove prefix (propagate)") {
  const char * const str = ")                                                                  ";
  for (i32_t i = 2; i <= (i32_t)::j::strlen(str); ++i) {
    for (i32_t j = 1; j < i; ++j) {
      landmark_array ar(make_landmark_array({str, i}));
      REQUIRE_EQ(ar.back().lbr_state_after().m_state, lbr_state::cp_sp);
      assert_remove_prefix(ar, {str, i}, j);
      REQUIRE_EQ(ar.back().lbr_state_after().m_state, lbr_state::sp);

    }
  }
}

TEST_CASE("Append (line breaks)") {
  for (i32_t i = 1; i < (i32_t)::j::strlen(g_lbr_string); ++i) {
    for (i32_t j = i + 1; j <= (i32_t)::j::strlen(g_lbr_string); ++j) {
      landmark_array ar(make_landmark_array({g_lbr_string, i}));
      landmark_change chg;
      ar.append(chg, {g_lbr_string, j}, {g_lbr_string + i, j - i});
      assert_lmk_array(ar, {g_lbr_string, j});
      REQUIRE_EQ(chg.length_delta, j - i);
      REQUIRE_EQ(chg.previous_lbr_state, compute_lbr_state({ g_lbr_string, i}));
      REQUIRE_EQ(chg.new_lbr_state, compute_lbr_state({ g_lbr_string, j}));
      landmark_array ar2(make_landmark_array({g_lbr_string, j}));
      REQUIRE_UNARY(ar == ar2);
    }
  }
}

TEST_CASE("Prepend (line breaks)") {
  for (i32_t prepend_sz = 1; prepend_sz < (i32_t)::j::strlen(g_lbr_string); ++prepend_sz) {
    for (i32_t original_sz = (i32_t)::j::strlen(g_lbr_string) - prepend_sz; original_sz > 0; --original_sz) {
      landmark_array ar(make_landmark_array({g_lbr_string + prepend_sz, original_sz}));
      landmark_change chg;
      ar.prepend(chg, {g_lbr_string, original_sz + prepend_sz}, {g_lbr_string, prepend_sz});
      assert_lmk_array(ar, {g_lbr_string, prepend_sz + original_sz});
      REQUIRE_EQ(chg.length_delta, prepend_sz);
      REQUIRE_EQ(chg.previous_lbr_state, compute_lbr_state({ g_lbr_string + prepend_sz, original_sz}));
      REQUIRE_EQ(chg.new_lbr_state, compute_lbr_state({ g_lbr_string, prepend_sz + original_sz}));
      landmark_array ar2(make_landmark_array({g_lbr_string, original_sz+prepend_sz}));
      REQUIRE_UNARY(ar == ar2);
    }
  }
}

TEST_CASE("Split (line breaks)") {
  for (i32_t sz = 2U; sz <= (i32_t)::j::strlen(g_lbr_string); ++sz) {
    const const_string_view sv{g_lbr_string, sz};
    for (i32_t split_point = 1U; split_point < sz; ++split_point) {
      landmark_array ar(make_landmark_array(sv));
      const const_string_view lhs{sv.prefix(split_point)};
      const const_string_view rhs{sv.without_prefix(split_point)};
      landmark_change chg;
      landmark_array ar2 = ar.split(chg, lhs, rhs);
      REQUIRE_EQ(chg.length_delta, -(i32_t)rhs.size());
      auto lbr = compute_lbr_state(lhs);
      REQUIRE_EQ(chg.hard_breaks_delta, -count_hard_breaks(rhs, lbr));
      REQUIRE_EQ(chg.new_lbr_state, lbr);
      assert_lmk_array(ar, lhs);
      assert_lmk_array(ar2, rhs, lbr);
    }
  }
}

TEST_SUITE_END();
