#include <detail/preamble.hpp>

#include "strings/unicode/rope/rope_utf8.hpp"
#include "strings/unicode/rope/marker.hpp"
#include "containers/pair.hpp"

TEST_SUITE_BEGIN("Strings - UTF-8 Rope Markers");

namespace s = j::strings;

namespace {
  using marker_info = j::pair<const s::marker *, u64_t>;
  void assert_markers(const s::rope_utf8 & r,
                      const marker_info * markers,
                      u32_t sz) {
    u32_t num = 0U;
    const s::marker * previous = nullptr;
    const s::rope_utf8_value * c = nullptr;
    auto check_chunk = [&](const s::marker & m) noexcept {
      ++num;
      REQUIRE_UNARY(m.previous() == previous);
      REQUIRE_UNARY(c == m.chunk());
      if (c->previous()) {
        REQUIRE_UNARY(m.offset() != 0 || m.binds_to_char_after());
      }
      previous = &m;
    };
    if (r.empty()) {
      c = &*r.chunks().begin();
      s::for_each_marker(*c, check_chunk);
    } else {
      for (auto & cc : r.chunks()) {
        previous = nullptr;
        c = &cc;
        s::for_each_marker(cc, check_chunk);
      }
    }
    REQUIRE(num == sz);
    for (u32_t i = 0; i < sz; ++i) {
      auto it = markers[i].first->as_byte_iterator();
      REQUIRE(it.position() == markers[i].second);
      bool did_find = false;
      if (r.empty()) {
        s::for_each_marker(*r.chunks().begin(), [&](const s::marker & m) noexcept {
          if (&m == markers[i].first) {
            did_find = true;
          }
        });
      } else {
        for (auto & c : r.chunks()) {
          s::for_each_marker(c, [&](const s::marker & m) noexcept {
            if (&m == markers[i].first) {
              did_find = true;
            }
          });
        }
      }
      REQUIRE_UNARY(did_find);

      for (u32_t j = 0; j < sz; ++j) {
        REQUIRE((markers[i].second < markers[j].second)
                == (*markers[i].first < *markers[j].first));
        REQUIRE(((i64_t)markers[i].second - (i64_t)markers[j].second)
                == (*markers[i].first - *markers[j].first));
      }
    }
  }

  template<u32_t N>
  void assert_markers(const s::rope_utf8 & r, const marker_info (& markers)[N]) {
    assert_markers(r, markers, N);
  }
}

TEST_CASE("Single char single marker") {
  s::rope_utf8 r("A");
  s::marker m(r.bytes().begin());
  assert_markers(r, {{&m, 0}});
  m.move(r.bytes().end());
  assert_markers(r, {{&m, 1}});
  m.invalidate();
  assert_markers(r, nullptr, 0);
}

TEST_CASE("Single char two markers") {
  s::rope_utf8 r("A");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin(), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}});
  m1.move(r.bytes().end());
  assert_markers(r, {{&m1, 1}, {&m2, 0}});
  m2.move(r.bytes().end());
  assert_markers(r, {{&m1, 1}, {&m2, 1}});
  SUBCASE("Invalidate first") {
    m2.invalidate();
    assert_markers(r, {{&m1, 1}});
  }
  SUBCASE("Invalidate second") {
    m1.invalidate();
    assert_markers(r, {{&m2, 1}});
  }
}

TEST_CASE("Single char three markers") {
  s::rope_utf8 r("A");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin());
  s::marker m3(r.bytes().begin());
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 0}});
  m2.move(r.bytes().end());
  assert_markers(r, {{&m1, 0}, {&m2, 1}, {&m3, 0}});
  SUBCASE("Invalidate first") {
    m1.invalidate();
    assert_markers(r, {{&m2, 1}, {&m3, 0}});
  }
  SUBCASE("Invalidate second") {
    m2.invalidate();
    assert_markers(r, {{&m1, 0}, {&m3, 0}});
  }
  SUBCASE("Invalidate third") {
    m3.invalidate();
    assert_markers(r, {{&m1, 0}, {&m2, 1}});
  }
}

TEST_CASE("Two chunks") {
  s::rope_utf8 r("Hello, darkness my old friend");
  u32_t sz = r.size_bytes();
  r.append("I've come to talk with you again");
  s::marker m1(r.bytes().iterate_at(sz - 1));
  s::marker m2(r.bytes().iterate_at(sz - 1), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(sz));
  s::marker m4(r.bytes().iterate_at(sz), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(sz + 1));
  s::marker m6(r.bytes().iterate_at(sz + 1), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, sz - 1}, {&m2, sz - 1}, {&m3, sz}, {&m4, sz}, {&m5, sz + 1}, {&m6, sz + 1}});
  auto it = r.chunks().begin();
  REQUIRE(m3.chunk() == &*it);
  ++it;
  REQUIRE(m4.chunk() == &*it);
}

TEST_CASE("Single chunk erase prefix") {
  s::rope_utf8 r("Hello");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(1));
  s::marker m4(r.bytes().iterate_at(1), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(2));
  s::marker m6(r.bytes().iterate_at(2), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(3));
  s::marker m8(r.bytes().iterate_at(3), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 1}, {&m4, 1}, {&m5, 2}, {&m6, 2}, {&m7, 3}, {&m8, 3}});
  r.erase(0, 2);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 0}, {&m4, 0}, {&m5, 0}, {&m6, 0}, {&m7, 1}, {&m8, 1}});
}

TEST_CASE("Single chunk erase suffix") {
  s::rope_utf8 r("Hello");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(3));
  s::marker m4(r.bytes().iterate_at(3), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(4));
  s::marker m6(r.bytes().iterate_at(4), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(5));
  s::marker m8(r.bytes().iterate_at(5), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 3}, {&m4, 3}, {&m5, 4}, {&m6, 4}, {&m7, 5}, {&m8, 5}});
  r.erase(3, 2);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 3}, {&m4, 3}, {&m5, 3}, {&m6, 3}, {&m7, 3}, {&m8, 3}});
}

TEST_CASE("Single chunk append in place") {
  s::rope_utf8 r("Hello");
  s::marker m1(r.bytes().iterate_at(4));
  s::marker m2(r.bytes().iterate_at(4), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(5));
  s::marker m4(r.bytes().iterate_at(5), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 4}, {&m2, 4}, {&m3, 5}, {&m4, 5}});
  r.append("World");
  assert_markers(r, {{&m1, 4}, {&m2, 4}, {&m3, 5}, {&m4, 10}});
}

TEST_CASE("Single chunk prepend in place") {
  s::rope_utf8 r("Hello");
  r.erase(0, 3);
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(1));
  s::marker m4(r.bytes().iterate_at(1), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 1}, {&m4, 1}});
  r.prepend("Jel");
  assert_markers(r, {{&m1, 0}, {&m2, 3}, {&m3, 4}, {&m4, 4}});
}

TEST_CASE("Single chunk append") {
  s::rope_utf8 r("Hello, darkness my old friend");
  u32_t sz = r.size_bytes();
  s::marker m1(r.bytes().iterate_at(sz - 1));
  s::marker m2(r.bytes().iterate_at(sz - 1), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().end());
  s::marker m4(r.bytes().end(), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, sz - 1}, {&m2, sz - 1}, {&m3, sz}, {&m4, sz}});
  r.append("I've come to talk with you again");
  assert_markers(r, {{&m1, sz - 1}, {&m2, sz - 1}, {&m3, sz}, {&m4, r.size_bytes()}});
}

TEST_CASE("Single chunk prepend") {
  s::rope_utf8 r("World");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(1));
  s::marker m4(r.bytes().iterate_at(1), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 1}, {&m4, 1}});
  r.prepend("Hello");
  assert_markers(r, {{&m1, 0}, {&m2, 5}, {&m3, 6}, {&m4, 6}});
}

TEST_CASE("Split") {
  s::rope_utf8 r("Hello, World!");
  s::marker m1(r.bytes().iterate_at(5));
  s::marker m2(r.bytes().iterate_at(5), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(6));
  s::marker m4(r.bytes().iterate_at(6), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(7));
  s::marker m6(r.bytes().iterate_at(7), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(9));
  s::marker m8(r.bytes().iterate_at(9), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 5}, {&m2, 5}, {&m3, 6}, {&m4, 6}, {&m5, 7}, {&m6, 7}, {&m7, 9}, {&m8, 9}});
  r.split(6);
  assert_markers(r, {{&m1, 5}, {&m2, 5}, {&m3, 6}, {&m4, 6}, {&m5, 7}, {&m6, 7}, {&m7, 9}, {&m8, 9}});
  auto it = r.chunks().begin();
  REQUIRE(m3.chunk() == &*it);
  ++it;
  REQUIRE(m4.chunk() == &*it);
}

TEST_CASE("Erase middle") {
  s::rope_utf8 r("Hello, World!");
  s::marker m1(r.bytes().iterate_at(4));
  s::marker m2(r.bytes().iterate_at(4), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(5));
  s::marker m4(r.bytes().iterate_at(5), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(6));
  s::marker m6(r.bytes().iterate_at(6), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(7));
  s::marker m8(r.bytes().iterate_at(7), s::marker_flag::binds_to_char_after);
  s::marker m9(r.bytes().iterate_at(8));
  s::marker m10(r.bytes().iterate_at(8), s::marker_flag::binds_to_char_after);
  s::marker m11(r.bytes().iterate_at(9));
  s::marker m12(r.bytes().iterate_at(9), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 4}, {&m2, 4}, {&m3, 5}, {&m4, 5}, {&m5, 6}, {&m6, 6}, {&m7, 7}, {&m8, 7}, {&m9, 8}, {&m10, 8}, {&m11, 9}, {&m12, 9}});
  r.erase(6, 2);
  assert_markers(r, {{&m1, 4}, {&m2, 4}, {&m3, 5}, {&m4, 5}, {&m5, 6}, {&m6, 6}, {&m7, 6}, {&m8, 6}, {&m9, 6}, {&m10, 6}, {&m11, 7}, {&m12, 7}});
}

TEST_CASE("Insert between") {
  s::rope_utf8 r("Hello, darkness my old friend");
  u32_t sz = r.size_bytes();
  r.append("I've come to talk with you again");
  s::marker m1(r.bytes().iterate_at(sz - 1));
  s::marker m2(r.bytes().iterate_at(sz - 1), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(sz));
  s::marker m4(r.bytes().iterate_at(sz), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(sz + 1));
  s::marker m6(r.bytes().iterate_at(sz + 1), s::marker_flag::binds_to_char_after);
  r.insert(sz, "----------");
  assert_markers(r, {{&m1, sz - 1}, {&m2, sz - 1}, {&m3, sz}, {&m4, sz + 10}, {&m5, sz + 11}, {&m6, sz + 11}});
}

TEST_CASE("Insert split") {
  s::rope_utf8 r("Hello, darkness my old friend");
  s::marker m1(r.bytes().iterate_at(9));
  s::marker m2(r.bytes().iterate_at(9), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(10));
  s::marker m4(r.bytes().iterate_at(10), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(11));
  s::marker m6(r.bytes().iterate_at(11), s::marker_flag::binds_to_char_after);
  r.insert(10, "----------");
  assert_markers(r, {{&m1, 9}, {&m2, 9}, {&m3, 10}, {&m4, 20}, {&m5, 21}, {&m6, 21}});
}

TEST_CASE("Empty") {
  s::rope_utf8 r;
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().end(), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}});
}

TEST_CASE("Empty insert") {
  s::rope_utf8 r;
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().end(), s::marker_flag::binds_to_char_after);
  r.append("Hello");
  assert_markers(r, {{&m1, 0}, {&m2, 5}});
}

TEST_CASE("Delete only") {
  s::rope_utf8 r("A");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().end(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().begin());
  s::marker m4(r.bytes().end(), s::marker_flag::binds_to_char_after);
  r.erase(0, 1);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 0}, {&m4, 0}});
}

TEST_CASE("Delete first") {
  s::rope_utf8 r("Hello, darkness my old friend");
  u32_t sz = r.size_bytes();
  r.append("-----------------");
  s::marker m1(r.bytes().begin());
  s::marker m2(r.bytes().begin(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(sz - 1));
  s::marker m4(r.bytes().iterate_at(sz - 1), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(sz));
  s::marker m6(r.bytes().iterate_at(sz), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(sz + 1));
  s::marker m8(r.bytes().iterate_at(sz + 1), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, sz - 1}, {&m4, sz - 1}, {&m5, sz}, {&m6, sz}, {&m7, sz + 1}, {&m8, sz + 1}});
  r.erase(0, sz);
  assert_markers(r, {{&m1, 0}, {&m2, 0}, {&m3, 0}, {&m4, 0}, {&m5, 0}, {&m6, 0}, {&m7, 1}, {&m8, 1}});
}

TEST_CASE("Delete second") {
  s::rope_utf8 r("Hello, darkness my old friend");
  u32_t sz = r.size_bytes();
  r.append("----------");
  s::marker m1(r.bytes().end());
  s::marker m2(r.bytes().end(), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(sz - 1));
  s::marker m4(r.bytes().iterate_at(sz - 1), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(sz));
  s::marker m6(r.bytes().iterate_at(sz), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(sz + 1));
  s::marker m8(r.bytes().iterate_at(sz + 1), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, sz + 10}, {&m2, sz + 10}, {&m3, sz - 1}, {&m4, sz - 1}, {&m5, sz}, {&m6, sz}, {&m7, sz + 1}, {&m8, sz + 1}});
  r.erase(sz, 10);
  assert_markers(r, {{&m1, sz}, {&m2, sz}, {&m3, sz - 1}, {&m4, sz - 1}, {&m5, sz}, {&m6, sz}, {&m7, sz}, {&m8, sz}});
}

TEST_CASE("Delete middle") {
  s::rope_utf8 r("Hello, darkness my old friend");
  u32_t sz = r.size_bytes();
  r.append("----------");
  r.append("I've come to talk with you again");
  s::marker m1(r.bytes().iterate_at(sz - 1));
  s::marker m2(r.bytes().iterate_at(sz - 1), s::marker_flag::binds_to_char_after);
  s::marker m3(r.bytes().iterate_at(sz));
  s::marker m4(r.bytes().iterate_at(sz), s::marker_flag::binds_to_char_after);
  s::marker m5(r.bytes().iterate_at(sz + 1));
  s::marker m6(r.bytes().iterate_at(sz + 1), s::marker_flag::binds_to_char_after);
  s::marker m7(r.bytes().iterate_at(sz + 9));
  s::marker m8(r.bytes().iterate_at(sz + 9), s::marker_flag::binds_to_char_after);
  s::marker m9(r.bytes().iterate_at(sz + 10));
  s::marker m10(r.bytes().iterate_at(sz + 10), s::marker_flag::binds_to_char_after);
  s::marker m11(r.bytes().iterate_at(sz + 11));
  s::marker m12(r.bytes().iterate_at(sz + 11), s::marker_flag::binds_to_char_after);
  assert_markers(r, {{&m1, sz - 1}, {&m2, sz - 1}, {&m3, sz}, {&m4, sz}, {&m5, sz + 1}, {&m6, sz + 1}, {&m7, sz + 9}, {&m8, sz + 9}, {&m9, sz + 10}, {&m10, sz + 10}, {&m11, sz + 11}, {&m12, sz + 11}});
  r.erase(sz, 10);
  assert_markers(r, {{&m1, sz - 1}, {&m2, sz - 1}, {&m3, sz}, {&m4, sz}, {&m5, sz}, {&m6, sz}, {&m7, sz}, {&m8, sz}, {&m9, sz}, {&m10, sz}, {&m11, sz + 1}, {&m12, sz + 1}});
  auto next = r.chunks().begin();
  ++next;
  REQUIRE(m3.chunk() == &*r.chunks().begin());
  REQUIRE(m4.chunk() == &*next);
  REQUIRE(m9.chunk() == &*r.chunks().begin());
  REQUIRE(m10.chunk() == &*next);
}

TEST_SUITE_END();
