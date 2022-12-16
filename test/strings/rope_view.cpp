#include "rope.hpp"

#include "strings/ropes/rope_view.hpp"

TEST_SUITE_BEGIN("Strings - Rope View");

namespace {
  template<typename View>
  void require_copy_from(const View & view, const char * expected, const sz_t sz) {
    const sz_t buf_sz = ::j::page_align_up(sz) + 2 * J_PAGE_SIZE;
    u8_t * buf = ::new u8_t[buf_sz];
    ::j::memset(buf, 0xAA, buf_sz);
    view.copy_to(buf + J_PAGE_SIZE, sz);
    for (sz_t i = 0; i < J_PAGE_SIZE; ++i) {
      if (J_UNLIKELY(buf[i] != 0xAA)) {
        FAIL("Overwrite before region");
      }
    }
    REQUIRE(::j::memcmp(buf + J_PAGE_SIZE, expected, sz) == 0);
    for (sz_t i = J_PAGE_SIZE + sz; i < buf_sz; ++i) {
      if (J_UNLIKELY(buf[i] != 0xAA)) {
        FAIL("Overwrite after region");
      }
    }
    ::delete[] buf;
  }

  template<typename View>
  void require_view_contents(const View & view, const char * expected, const sz_t sz) {
    REQUIRE(view.empty() == !sz);
    REQUIRE(!view == !sz);
    REQUIRE((bool)view == (bool)sz);
    REQUIRE(view.size() == sz);
    const auto begin = view.begin();
    auto it = begin;
    const auto end = view.end();
    for (sz_t i = 0; i < sz; ++i, ++it) {
      REQUIRE_UNARY(it != end);
      REQUIRE(it - begin == i);
      REQUIRE(*it == expected[i]);
      REQUIRE(*(begin + i) == expected[i]);
    }
    REQUIRE_UNARY(it == end);
    if (sz) {
      require_copy_from(view, expected, sz);
    }
  }

  template<typename View>
  void require_view_contents(const View & view, const char * expected) {
    require_view_contents(view, expected, ::j::strlen(expected));
  }
}

TEST_CASE_TEMPLATE("Empty", View, s::binary_rope_view) {
  View v;
  require_view_contents(v, "");
}

TEST_CASE_TEMPLATE("Single character", View, s::binary_rope_view) {
  s::binary_rope r{"A"};
  View v{r};
  require_view_contents(v, "A");
}

TEST_CASE_TEMPLATE("Multiple characters, one chunk", View, s::binary_rope_view) {
  s::binary_rope r{"Hello, World!"};
  View v{r};
  require_view_contents(v, "Hello, World!");
}

TEST_CASE_TEMPLATE("Two chunks", View, s::binary_rope_view) {
  s::binary_rope r{"a"};
  r += "b";
  View v{r};
  require_view_contents(v, "ab");
}

TEST_CASE_TEMPLATE("Three chunks", View, s::binary_rope_view) {
  s::binary_rope r{"a"};
  r += "b";
  r += "c";
  View v{r};
  require_view_contents(v, "abc");
}

TEST_CASE_TEMPLATE("Start of a single chunk", View, s::binary_rope_view) {
  s::binary_rope r{"Hello, World!"};
  View v{r.view_at(0, 5)};
  require_view_contents(v, "Hello");
}

TEST_CASE_TEMPLATE("End of a single chunk", View, s::binary_rope_view) {
  s::binary_rope r{"Hello, World!"};
  View v{r.view_at(7, 6)};
  require_view_contents(v, "World!");
}

TEST_CASE_TEMPLATE("Middle of a single chunk", View, s::binary_rope_view) {
  s::binary_rope r{"Hello, World!"};
  View v{r.view_at(3, 6)};
  require_view_contents(v, "lo, Wo");
}

TEST_CASE_TEMPLATE("Middle of two chunks", View, s::binary_rope_view) {
  s::binary_rope r{"Hello,"};
  r += " World!";
  View v{r.view_at(2, 7)};
  require_view_contents(v, "llo, Wo");
}

TEST_CASE_TEMPLATE("All combinations", View, s::binary_rope_view) {
  const char * const str = "Hello, World, and Goodbye.";
  const sz_t len = ::j::strlen(str);
  s::binary_rope r{"Hello,"};
  r += " World,";
  r += " and ";
  r += "Goodbye.";
  for (sz_t start = 0U; start < len; ++start) {
    for (sz_t sz = 0U; start + sz < len; ++sz) {
      View v{r.view_at(start, sz)};
      require_view_contents(v, str + start, sz);
    }
  }
}

TEST_CASE("Drop/take front") {
  s::binary_rope r{"Hello"};
  r += ", ";
  r += "World!";
  s::binary_rope_view v{r};
  SUBCASE("Zero") {
    SUBCASE("Drop") { v.drop_front(0U); }
    SUBCASE("Take") { require_view_contents(v.take_front(0U), ""); }
    require_view_contents(v, "Hello, World!");
  }
  SUBCASE("One by one") {
    const char * const str = "Hello, World!";
    SUBCASE("Drop") {
      for (u32_t i = 1; i <= 13; ++i) {
        v.drop_front(1U);
        require_view_contents(v, str + i, 13U - i);
      }
    }
    SUBCASE("Take") {
      for (u32_t i = 1; i <= 13; ++i) {
        require_view_contents(v.take_front(1U), str + i - 1, 1U);
        require_view_contents(v, str + i, 13U - i);
      }
    }
  }
  SUBCASE("First chunk") {
    SUBCASE("Drop") { v.drop_front(5U); }
    SUBCASE("Take") { require_view_contents(v.take_front(5U), "Hello"); }
    require_view_contents(v, ", World!");
  }
  SUBCASE("Second chunk") {
    SUBCASE("Drop") { v.drop_front(7U); }
    SUBCASE("Take") { require_view_contents(v.take_front(7U), "Hello, "); }
    require_view_contents(v, "World!");
  }
  SUBCASE("Mid-second chunk") {
    SUBCASE("Drop") { v.drop_front(6U); }
    SUBCASE("Take") { require_view_contents(v.take_front(6U), "Hello,"); }
    require_view_contents(v, " World!");
  }
  SUBCASE("Mid-third chunk") {
    SUBCASE("Drop") { v.drop_front(9U); }
    SUBCASE("Take") { require_view_contents(v.take_front(9U), "Hello, Wo"); }
    require_view_contents(v, "rld!");
  }
  SUBCASE("Completely") {
    SUBCASE("Drop") { v.drop_front(13U); }
    SUBCASE("Take") { require_view_contents(v.take_front(13U), "Hello, World!"); }
    require_view_contents(v, "");
  }
}

TEST_CASE("Drop/take back") {
  s::binary_rope r{"Hello"};
  r += ", ";
  r += "World!";
  s::binary_rope_view v{r};
  SUBCASE("Zero") {
    SUBCASE("Drop") {
      v.drop_back(0U);
    }
    SUBCASE("Take") {
      require_view_contents(v.take_back(0U), "");
    }
    require_view_contents(v, "Hello, World!");
  }
  SUBCASE("One by one") {
    const char * const str = "Hello, World!";
    SUBCASE("Drop") {
      for (u32_t i = 1; i <= 13; ++i) {
        v.drop_back(1U);
        require_view_contents(v, str, 13U - i);
      }
    }
    SUBCASE("Take") {
      for (u32_t i = 1; i <= 13; ++i) {
        require_view_contents(v.take_back(1U), str + 13U - i, 1U);
        require_view_contents(v, str, 13U - i);
      }
    }
  }
  SUBCASE("Last chunk") {
    SUBCASE("Drop") { v.drop_back(6U); }
    SUBCASE("Take") { require_view_contents(v.take_back(6U), "World!"); }
    require_view_contents(v, "Hello, ");
  }
  SUBCASE("Second chunk") {
    SUBCASE("Drop") { v.drop_back(8U); }
    SUBCASE("Take") { require_view_contents(v.take_back(8U), ", World!"); }
    require_view_contents(v, "Hello");
  }
  SUBCASE("Mid-second chunk") {
    SUBCASE("Drop") { v.drop_back(7U); }
    SUBCASE("Take") { require_view_contents(v.take_back(7U), " World!"); }
    require_view_contents(v, "Hello,");
  }
  SUBCASE("Mid-first chunk") {
    SUBCASE("Drop") { v.drop_back(10U); }
    SUBCASE("Take") { require_view_contents(v.take_back(10U), "lo, World!"); }
    require_view_contents(v, "Hel");
  }
  SUBCASE("Completely") {
    SUBCASE("Drop") { v.drop_back(13U); }
    SUBCASE("Take") { require_view_contents(v.take_back(13U), "Hello, World!"); }
    require_view_contents(v, "");
  }
}

TEST_SUITE_END();
