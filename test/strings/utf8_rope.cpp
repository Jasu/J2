#include <detail/preamble.hpp>

#include "strings/unicode/utf8_rope.hpp"
#include "strings/unicode/utf8_rope_debug.hpp"
#include "containers/vector.hpp"
#include "rope.hpp"

namespace {
  template<u32_t N>
  void require_utf8_rope_chunks(const s::utf8_rope & r, const char * const (& chunks)[N]) {
    require_rope_chunks(r.raw(), chunks);
  }

  void require_empty_utf8_rope(const s::utf8_rope & r) {
    require_empty_rope(r.raw());
  }

  void require_utf8_rope_contents(const s::utf8_rope & r, const char * contents, sz_t sz) {
    require_rope_contents(r.raw(), contents, sz);
    J_ASSERT_UTF8_ROPE(r);
  }

  void require_utf8_rope_contents(const s::utf8_rope & r, const char * contents) {
    require_utf8_rope_contents(r, contents, ::j::strlen(contents));
  }
}

TEST_SUITE_BEGIN("Strings - UTF-8 Rope");

namespace s = j::strings;

TEST_CASE("Empty") {
  s::utf8_rope r;
  require_empty_utf8_rope(r);
}

TEST_CASE("Single valid") {
  s::utf8_rope r;
  r.insert(0U, "Hello");
  require_utf8_rope_contents(r, "Hello");
  require_utf8_rope_chunks(r, {"Hello"});
}

TEST_CASE("Single invalid") {
  const char * c = nullptr;
  SUBCASE("1 trailing leader")  { c = "Hello\x80"; }
  SUBCASE("2 trailing leaders") { c = "Hello\x80\x80"; }
  SUBCASE("3 trailing leaders") { c = "Hello\x80\x80\x80"; }
  SUBCASE("4 trailing leaders") { c = "Hello\x80\x80\x80\x80"; }
  SUBCASE("1 leading leader")   { c = "\x80Hello"; }
  SUBCASE("2 leading leaders")  { c = "\x80\x80Hello"; }
  SUBCASE("3 leading leaders")  { c = "\x80\x80\x80Hello"; }
  SUBCASE("4 leading leaders")  { c = "\x80\x80\x80\x80Hello"; }
  SUBCASE("1 leader")           { c = "\x80"; }
  SUBCASE("2 leaders")          { c = "\x80\x80"; }
  SUBCASE("3 leaders")          { c = "\x80\x80\x80"; }
  SUBCASE("4 leaders")          { c = "\x80\x80\x80\x80"; }
  SUBCASE("Truncated end 1")    { c = "A\xC2"; }
  SUBCASE("Truncated end 2")    { c = "A\xE0"; }
  SUBCASE("Truncated end 2.1")  { c = "A\xE0\x80"; }
  SUBCASE("Truncated end 3")    { c = "A\xF0"; }
  SUBCASE("Truncated end 3.1")  { c = "A\xF0\x80"; }
  SUBCASE("Truncated end 3.2")  { c = "A\xF0\x80\x80"; }
  SUBCASE("Truncated only 1")   { c = "\xC2"; }
  SUBCASE("Truncated only 2")   { c = "\xE0"; }
  SUBCASE("Truncated only 2.1") { c = "\xE0\x80"; }
  SUBCASE("Truncated only 3")   { c = "\xF0"; }
  SUBCASE("Truncated only 3.1") { c = "\xF0\x80"; }
  SUBCASE("Truncated only 3.2") { c = "\xF0\x80\x80"; }
  SUBCASE("Truncated lead 1")   { c = "\x80\xC2"; }
  SUBCASE("Truncated lead 2")   { c = "\x80\xE0"; }
  SUBCASE("Truncated lead 2.1") { c = "\x80\xE0\x80"; }
  SUBCASE("Truncated lead 3")   { c = "\x80\xF0"; }
  SUBCASE("Truncated lead 3.1") { c = "\x80\xF0\x80"; }
  SUBCASE("Truncated lead 3.2") { c = "\x80\xF0\x80\x80"; }

  s::utf8_rope r;
  r.insert(0U, c);
  require_utf8_rope_contents(r, c);
  require_utf8_rope_chunks(r, {c});

  s::utf8_rope r2;
  r2 += c;
  require_utf8_rope_contents(r2, c);
  require_utf8_rope_chunks(r2, {c});
}

TEST_CASE("Two valid") {
  s::utf8_rope r;
  SUBCASE("In order") {
    r.insert(0U, "Hello");
    r.insert(5U, ", World");
  }
  SUBCASE("Reverse") {
    r.insert(0U, ", World");
    r.insert(0U, "Hello");
  }
  SUBCASE("Append") {
    r += "Hello";
    r += ", World";
  }
  require_utf8_rope_contents(r, "Hello, World");
  require_utf8_rope_chunks(r, {"Hello", ", World"});
}

TEST_CASE("Three valid") {
  s::utf8_rope r;
  SUBCASE("In order") {
    r.insert(0U, "Hello");
    r.insert(5U, ", ");
    r.insert(7U, "World");
  }
  SUBCASE("Reverse") {
    r.insert(0U, "World");
    r.insert(0U, ", ");
    r.insert(0U, "Hello");
  }
  SUBCASE("Append") {
    r += "Hello";
    r += ", ";
    r += "World";
  }
  require_utf8_rope_contents(r, "Hello, World");
  require_utf8_rope_chunks(r, {"Hello", ", ", "World"});
}

TEST_CASE("Invalid combine") {
  s::utf8_rope r;
  const char * expected = nullptr;
  SUBCASE("Successors combine 1") {
    r.insert(0U, "\x81");
    r.insert(1U, "\x82");
    expected = "\x81\x82";
  }
  SUBCASE("Successors combine 1 append") {
    r += "\x81";
    r += "\x82";
    expected = "\x81\x82";
  }
  SUBCASE("Successors combine 2") {
    r.insert(0U, "\x81");
    r.insert(0U, "\x82");
    expected = "\x82\x81";
  }
  SUBCASE("Successors combine 3") {
    r.insert(0U, "\x81");
    r.insert(0U, "A\x82");
    expected = "A\x82\x81";
  }
  SUBCASE("Successors combine 4") {
    r.insert(0U, "\x81");
    r.insert(1U, "\x82""A");
    expected = "\x81\x82""A";
  }
  SUBCASE("Successors combine 4 append") {
    r += "\x81";
    r += "\x82""A";
    expected = "\x81\x82""A";
  }
  SUBCASE("Add trailing 1") {
    r.insert(0U, "\xC2");
    r.insert(1U, "\x82");
    expected = "\xC2\x82";
  }
  SUBCASE("Add trailing 1 append") {
    r += "\xC2";
    r += "\x82";
    expected = "\xC2\x82";
  }
  SUBCASE("Add trailing 2.0") {
    r.insert(0U, "\xE1");
    r.insert(1U, "\x82\x81");
    expected = "\xE1\x82\x81";
  }
  SUBCASE("Add trailing 2.0 append") {
    r += "\xE1";
    r += "\x82\x81";
    expected = "\xE1\x82\x81";
  }
  SUBCASE("Add trailing 2.1") {
    r.insert(0U, "\xE1");
    r.insert(1U, "\x82");
    expected = "\xE1\x82";
  }
  SUBCASE("Add trailing 2.1 append") {
    r += "\xE1";
    r += "\x82";
    expected = "\xE1\x82";
  }
  SUBCASE("Add trailing 2.2") {
    r.insert(0U, "\xE1");
    r.insert(1U, "\x82");
    r.insert(2U, "\x81");
    expected = "\xE1\x82\x81";
  }
  SUBCASE("Add trailing 2.2 append") {
    r += "\xE1";
    r += "\x82";
    r += "\x81";
    expected = "\xE1\x82\x81";
  }
  SUBCASE("Add trailing 3.0") {
    r.insert(0U, "\xF1");
    r.insert(1U, "\x82\x81\x83");
    expected = "\xF1\x82\x81\x83";
  }
  SUBCASE("Add trailing 3.0 append") {
    r += "\xF1";
    r += "\x82\x81\x83";
    expected = "\xF1\x82\x81\x83";
  }
  SUBCASE("Add trailing 3.1") {
    r.insert(0U, "\xF1");
    r.insert(1U, "\x82");
    expected = "\xF1\x82";
  }
  SUBCASE("Add trailing 3.1 append") {
    r += "\xF1";
    r += "\x82";
    expected = "\xF1\x82";
  }
  SUBCASE("Add trailing 3.2") {
    r.insert(0U, "\xF1");
    r.insert(1U, "\x82");
    r.insert(2U, "\x83");
    expected = "\xF1\x82\x83";
  }
  SUBCASE("Add trailing 3.2 append") {
    r += "\xF1";
    r += "\x82";
    r += "\x83";
    expected = "\xF1\x82\x83";
  }
  SUBCASE("Add trailing 3.3") {
    r.insert(0U, "\xF1");
    r.insert(1U, "\x82");
    r.insert(2U, "\x83");
    r.insert(3U, "\x84");
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add trailing 3.3 append") {
    r += "\xF1";
    r += "\x82";
    r += "\x83";
    r += "\x84";
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add trailing 3.4") {
    r.insert(0U, "\xF1");
    r.insert(1U, "\x82\x83");
    r.insert(3U, "\x84");
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add trailing 3.4 append") {
    r += "\xF1";
    r += "\x82\x83";
    r += "\x84";
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add trailing 3.5") {
    r.insert(0U, "\xF1\x82");
    r.insert(2U, "\x83\x84");
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add trailing 3.5 append") {
    r += "\xF1\x82";
    r += "\x83\x84";
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add leading 1") {
    r.insert(0U, "\x82");
    r.insert(0U, "\xC2");
    expected = "\xC2\x82";
  }
  SUBCASE("Add leading 2.0") {
    r.insert(0U, "\x82\x81");
    r.insert(0U, "\xE1");
    expected = "\xE1\x82\x81";
  }
  SUBCASE("Add leading 2.1") {
    r.insert(0U, "\x82");
    r.insert(0U, "\xE1");
    expected = "\xE1\x82";
  }
  SUBCASE("Add leading 2.2") {
    r.insert(0U, "\x81");
    r.insert(0U, "\x82");
    r.insert(0U, "\xE1");
    expected = "\xE1\x82\x81";
  }
  SUBCASE("Add leading 3.0") {
    r.insert(0U, "\x82\x81\x83");
    r.insert(0U, "\xF1");
    expected = "\xF1\x82\x81\x83";
  }
  SUBCASE("Add leading 3.1") {
    r.insert(0U, "\x82");
    r.insert(0U, "\xF1");
    expected = "\xF1\x82";
  }
  SUBCASE("Add leading 3.2") {
    r.insert(0U, "\x83");
    r.insert(0U, "\x82");
    r.insert(0U, "\xF1");
    expected = "\xF1\x82\x83";
  }
  SUBCASE("Add leading 3.3") {
    r.insert(0U, "\x84");
    r.insert(0U, "\x83");
    r.insert(0U, "\x82");
    r.insert(0U, "\xF1");
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add leading 3.4") {
    r.insert(0U, "\x84");
    r.insert(0U, "\x82\x83");
    r.insert(0U, "\xF1");
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add leading 3.5") {
    r.insert(0U, "\x83\x84");
    r.insert(0U, "\xF1\x82");
    expected = "\xF1\x82\x83\x84";
  }
  SUBCASE("Add middle 2") {
    r.insert(0U, "\xE1\x82");
    r.insert(1U, "\x80");
    expected = "\xE1\x80\x82";
  }
  SUBCASE("Add middle 3") {
    r.insert(0U, "\xF1\x82");
    r.insert(1U, "\x83\x84");
    expected = "\xF1\x83\x84\x82";
  }
  SUBCASE("Add middle 4") {
    r.insert(0U, "\xE1\x82");
    r.insert(1U, "\x80\x81\xC2");
    expected = "\xE1\x80\x81\xC2\x82";
  }
  SUBCASE("Add middle 5") {
    r.insert(0U, "\xE1\x82");
    r.insert(1U, "\x80\xC2");
    expected = "\xE1\x80\xC2\x82";
  }
  require_utf8_rope_contents(r, expected);
  require_utf8_rope_chunks(r, {expected});
}

TEST_CASE("Invalid combine two chunks") {
  s::utf8_rope r;
  const char * expected = nullptr, * expected1 = nullptr, *expected2 = nullptr;
  SUBCASE("No combine 1") {
    r.insert(0U, expected1 = "\x80""A");
    r.insert(2U, expected2 = "\x81""B");
    expected = "\x80""A\x81""B";
  }
  SUBCASE("No combine 1 append") {
    r += (expected1 = "\x80""A");
    r += (expected2 = "\x81""B");
    expected = "\x80""A\x81""B";
  }
  SUBCASE("No combine 1.1") {
    r.insert(0U, expected2 = "\x81""B");
    r.insert(0U, expected1 = "\x80""A");
    expected = "\x80""A\x81""B";
  }
  SUBCASE("No combine 2") {
    r.insert(0U, expected1 = "\x80");
    r.insert(1U, expected2 = "B\x87");
    expected = "\x80""B\x87";
  }
  SUBCASE("No combine 2 append") {
    r += (expected1 = "\x80");
    r += (expected2 = "B\x87");
    expected = "\x80""B\x87";
  }
  SUBCASE("No combine 2.1") {
    r.insert(0U, expected2 = "B\x87");
    r.insert(0U, expected1 = "\x80");
    expected = "\x80""B\x87";
  }
  SUBCASE("No combine 3") {
    r.insert(0U, expected1 = "\xF0");
    r.insert(1U, expected2 = "B\x87");
    expected = "\xF0""B\x87";
  }
  SUBCASE("No combine 3 append") {
    r += (expected1 = "\xF0");
    r += (expected2 = "B\x87");
    expected = "\xF0""B\x87";
  }
  SUBCASE("No combine 3.1") {
    r.insert(0U, expected2 = "B\x87");
    r.insert(0U, expected1 = "\xF0");
    expected = "\xF0""B\x87";
  }
  SUBCASE("Steal before 1") {
    r.insert(0U, "A\xF0");
    r.insert(2U, "\x80");
    expected = "A\xF0\x80";
    expected1 = "A"; expected2 = "\xF0\x80";
  }
  SUBCASE("Steal before 1 append") {
    r +="A\xF0";
    r +="\x80";
    expected = "A\xF0\x80";
    expected1 = "A"; expected2 = "\xF0\x80";
  }
  SUBCASE("Steal before 1.1") {
    r.insert(0U, "A\xF0");
    r.insert(2U, "\x80""B");
    expected = "A\xF0\x80""B";
    expected1 = "A"; expected2 = "\xF0\x80""B";
  }
  SUBCASE("Steal before 1.1 append") {
    r += "A\xF0";
    r += "\x80""B";
    expected = "A\xF0\x80""B";
    expected1 = "A"; expected2 = "\xF0\x80""B";
  }
  SUBCASE("Steal after 1") {
    r.insert(0U, "\x80""A");
    r.insert(0U, "\xF0");
    expected = "\xF0\x80""A";
    expected1 = "\xF0\x80"; expected2 = "A";
  }
  SUBCASE("Steal after 1.1") {
    r.insert(0U, "\x80""A");
    r.insert(0U, "B\xF0");
    expected = "B\xF0\x80""A";
    expected1 = "B\xF0\x80"; expected2 = "A";
  }
  require_utf8_rope_contents(r, expected);
  require_utf8_rope_chunks(r, {expected1, expected2});
}

TEST_CASE("Invalid combine three chunks") {
  s::utf8_rope r;
  const char * expected = nullptr, * expected1 = nullptr, *expected2 = nullptr, *expected3 = nullptr;
  SUBCASE("No combine 1") {
    r.insert(0U, "AB");
    r.insert(1U, expected2 = "\x81");
    expected = "A\x81""B";
    expected1 = "A", expected3 = "B";
  }
  SUBCASE("No combine 2") {
    r.insert(0U, "AB");
    r.insert(1U, expected2 = "\xF1");
    expected = "A\xF1""B";
    expected1 = "A", expected3 = "B";
  }
  SUBCASE("No combine 3") {
    r.insert(0U, "\x80\xF8");
    r.insert(1U, expected2 = "\xF1");
    expected = "\x80\xF1\xF8";
    expected1 = "\x80", expected3 = "\xF8";
  }
  SUBCASE("Steal both") {
    r.insert(0U, "A\xF0\x80""B");
    r.insert(2U, "\x81");
    expected = "A\xF0\x81\x80""B";
    expected1 = "A", expected2 = "\xF0\x81\x80", expected3 = "B";
  }
  SUBCASE("Steal partial") {
    r.insert(0U, "A\xF0\x81\x80");
    r.insert(2U, "\xC2");
    expected = "A\xF0\xC2\x81\x80";
    expected1 = "A\xF0", expected2 = "\xC2\x81", expected3 = "\x80";
  }
  require_utf8_rope_contents(r, expected);
  require_utf8_rope_chunks(r, {expected1, expected2, expected3});
}

TEST_CASE("Erase full") {
  s::utf8_rope r;
  SUBCASE("Single chunk") {
    r += "Hello, world";
  }
  SUBCASE("Two chunks") {
    r += "Hello";
    r += ", world";
  }
  SUBCASE("Three chunks") {
    r += "Hello";
    r += ", ";
    r += "world";
  }
  r.erase(0, 12);
  require_empty_utf8_rope(r);
}

TEST_CASE("Erase all but one, valid") {
  s::utf8_rope r;
  SUBCASE("End") {
    r += "Hello, world";
    r.erase(5, 7);
  }
  SUBCASE("End 2") {
    r += "Hello";
    r += ", world";
    r.erase(5, 7);
  }
  SUBCASE("End 3") {
    r += "Hello, ";
    r += "world";
    r.erase(5, 7);
  }
  SUBCASE("End 4") {
    r += "Hello";
    r += ", ";
    r += "world";
    r.erase(5, 7);
  }
  SUBCASE("Begin") {
    r += "World, Hello";
    r.erase(0, 7);
  }
  SUBCASE("Begin 2") {
    r += "World, ";
    r += "Hello";
    r.erase(0, 7);
  }
  SUBCASE("Begin 3") {
    r += "World";
    r += ", Hello";
    r.erase(0, 7);
  }
  SUBCASE("Begin 4") {
    r += "World";
    r += ", ";
    r+= "Hello";
    r.erase(0, 7);
  }
  require_utf8_rope_contents(r, "Hello");
  require_utf8_rope_chunks(r, {"Hello"});
}

TEST_CASE("Erase middle, valid") {
  s::utf8_rope r;
  SUBCASE("Single") {
    r += "Hello, World";
    r.erase(3, 6);
  }
  SUBCASE("Two") {
    r += "Hello,";
    r += " World";
    r.erase(3, 6);
  }
  SUBCASE("Three") {
    r += "Hello";
    r += ", ";
    r += "World";
    r.erase(3, 6);
  }
  SUBCASE("Boundaries") {
    r += "Hel";
    r += "lo, Wo";
    r += "rld";
    r.erase(3, 6);
  }
  require_utf8_rope_contents(r, "Helrld");
  require_utf8_rope_chunks(r, {"Hel", "rld"});
}

TEST_CASE("Erase middle, combine") {
  s::utf8_rope r;
  const char * expected = nullptr;
  SUBCASE("Make valid 1") {
    r += "\xC2""A\x82";
    expected = "\xC2\x82";
    r.erase(1, 1);
  }
  SUBCASE("Make valid 2") {
    r += "\xC2""A";
    r += "\x82";
    expected = "\xC2\x82";
    r.erase(1, 1);
  }
  SUBCASE("Make valid 3") {
    r += "\xC2";
    r += "A\x82";
    expected = "\xC2\x82";
    r.erase(1, 1);
  }
  SUBCASE("Make valid 4") {
    r += "\xC2";
    r += "A";
    r += "\x82";
    expected = "\xC2\x82";
    r.erase(1, 1);
  }
  SUBCASE("Make valid 5") {
    r += "\xC2""A";
    r += "B";
    r += "C\x82";
    expected = "\xC2\x82";
    r.erase(1, 3);
  }

  SUBCASE("Combine leaders") {
    r += "\x82""A\x83";
    expected = "\x82\x83";
    r.erase(1, 1);
  }
  SUBCASE("Combine leaders 2") {
    r += "\x82";
    r += "A\x83";
    expected = "\x82\x83";
    r.erase(1, 1);
  }
  SUBCASE("Combine leaders 3") {
    r += "\x82""A";
    r += "\x83";
    expected = "\x82\x83";
    r.erase(1, 1);
  }
  SUBCASE("Combine leaders 3") {
    r += "\x82";
    r += "A";
    r += "\x83";
    expected = "\x82\x83";
    r.erase(1, 1);
  }
  SUBCASE("Combine leaders 5") {
    r += "\x82""A";
    r += "B";
    r += "C\x83";
    expected = "\x82\x83";
    r.erase(1, 3);
  }

  SUBCASE("Valid to valid") {
    r += "\xC2\x81\xC3\x82";
    expected = "\xC2\x82";
    r.erase(1, 2);
  }
  require_utf8_rope_contents(r, expected);
  require_utf8_rope_chunks(r, {expected});
}

TEST_CASE("Erase middle, no combine") {
  s::utf8_rope r;
  const char *expected = nullptr, *expected1 = nullptr, *expected2 = nullptr;
  SUBCASE("Successor before") {
    r += "\x82""A\xC2";
    expected = "\x82\xC2", expected1 = "\x82", expected2 = "\xC2";
    r.erase(1, 1);
  }
  SUBCASE("Valid before") {
    r += "\xC2\x82""A\x83";
    expected = "\xC2\x82\x83", expected1 = "\xC2\x82", expected2 = "\x83";
    r.erase(2, 1);
  }
  SUBCASE("Valid after") {
    r += "\xC2""A\xC3\x80";
    expected = "\xC2\xC3\x80", expected1 = "\xC2", expected2 = "\xC3\x80";
    r.erase(1, 1);
  }
  require_utf8_rope_contents(r, expected);
  require_utf8_rope_chunks(r, {expected1, expected2});
}

namespace {
  void add_ascii(j::vector<u32_t> & to, const char * from) {
    while (*from) { to.push_back(*from++); }
  }
}

TEST_CASE("Iteration, valid") {
  s::utf8_rope r;
  using v = j::vector<u32_t>;
  v expected;

  SUBCASE("Empty") { }
  SUBCASE("Ascii single") {
    r += "Hello";
    add_ascii(expected, "Hello");
  }
  SUBCASE("Ascii two") {
    r += "Hel";
    r += "lo";
    add_ascii(expected, "Hello");
  }
  SUBCASE("Ascii five") {
    r += "H"; r += "e"; r += "l"; r += "l"; r += "o";
    add_ascii(expected, "Hello");
  }
  SUBCASE("Two byte (IKEA)") {
    r += "Ĺÿķťfîƀƀļä";
    expected.push_back(0x139);
    expected.push_back(0xFF);
    expected.push_back(0x137);
    expected.push_back(0x165);
    expected.push_back('f');
    expected.push_back(0xEE);
    expected.push_back(0x180);
    expected.push_back(0x180);
    expected.push_back(0x13C);
    expected.push_back(0xE4);

  }
  SUBCASE("Three byte NUL") {
    r += "␀";
    expected.push_back(0x2400);
  }
  SUBCASE("Three byte SYN flood") {
    SUBCASE("Whole") {
      r += "␖␖␖␖␖";
    }
    SUBCASE("Two") {
      r += "␖␖"; r += "␖␖␖";
    }
    SUBCASE("Separate") {
      for (int i = 0; i < 5; ++i) {
        r += "␖";
      }
    }
    for (int i = 0; i < 5; ++i) {
      expected.push_back(0x2416);
    }
  }
  SUBCASE("Four byte (Red dragon)") {
    r += "🀄";
    expected.push_back(0x1f004);
  }
  SUBCASE("Four byte (Nine gates of bamboos) ") {
    SUBCASE("Whole") {
      r += "🀐🀐🀐🀑🀒🀓🀔🀕🀖🀗🀘🀘🀘 🀕";
    }
    SUBCASE("Two") {
      r += "🀐🀐🀐🀑🀒🀓🀔🀕🀖🀗🀘🀘🀘"; r += " 🀕";
    }
    SUBCASE("Separate") {
      r += "🀐"; r += "🀐"; r += "🀐"; r += "🀑"; r += "🀒"; r += "🀓"; r += "🀔"; r += "🀕";
      r += "🀖"; r += "🀗"; r += "🀘"; r += "🀘"; r += "🀘"; r += " "; r += "🀕";
    }
    for (u32_t i = 0; i < 9; ++i) {
      for (u32_t j = 0; j < ((i == 0 || i == 8) ? 3 : 1); ++j) {
        expected.push_back(0x1f010 + i);
      }
    }
    expected.push_back(' ');
    expected.push_back(0x1f015);
  }
  SUBCASE("Four byte (Private use)") {
    r += "\xF0\x9F\xBC\x80";
    expected.push_back(0x1ff00);
  }
  SUBCASE("Valid edges") {
    r.append("", 1); expected.push_back(0U);
    r += "\x7F"; expected.push_back(0x7FU);
    r += "\xC2\x80"; expected.push_back(0x80U);
    r += "\xDF\xBF"; expected.push_back(0x7FFU);
    r += "\xE0\xA0\x80"; expected.push_back(0x800);
    r += "\xED\x9F\xBF"; expected.push_back(0xD7FF);
    r += "\xEE\x80\x80"; expected.push_back(0xE000);
    r += "\xEF\xBF\xBF"; expected.push_back(0xFFFF);
    r += "\xF0\x90\x80\x80"; expected.push_back(0x10000);
    r += "\xF4\x8F\xBF\xBF"; expected.push_back(0x10FFFF);
  }

  u32_t i = 0;
  for (auto codepoint : r) {
    REQUIRE(expected.at(i++) == codepoint);
  }
  REQUIRE(i == expected.size());
}

TEST_CASE("Iteration, invalid") {
  s::utf8_rope r;
  using v = j::vector<u32_t>;
  v expected;
  SUBCASE("Lone successor") {
    r+= "\x80";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("Two lone successors") {
    r+= "\x80\xBF";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("Three lone successors") {
    r+= "\x80\x80\xBF";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("C0") {
    r+= "\xC0\x80";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("C1") {
    r+= "\xC1\xBF";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("C0 valid after") {
    r+= "\xC0\x81""A";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("C1 valid after") {
    r+= "\xC1\xBF""A";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("E080") {
    r+= "\xE0\x80\x80";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("E09F") {
    r+= "\xE0\x9F\xBF";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("E09F Valid after") {
    r+= "\xE0\x9F\xBF""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("EDA0") {
    r+= "\xED\xA0\x80";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("EDA0 Valid after") {
    r+= "\xED\xA0\x80""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("F080") {
    r+= "\xF0\x80\x80\x80";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("F08F") {
    r+= "\xF0\x8F\xBF\xBF";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("F08F Valid after") {
    r+= "\xF0\x8F\xBF\xBF""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("F490") {
    r+= "\xF4\x90\x80\x80";
    expected.push_back(0xFFFDU);
  }
  SUBCASE("F490 Valid after") {
    r+= "\xF4\x90\x80\x80""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("F580") {
    r+= "\xF5\x80\x80\x80";
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("Truncated 1") {
    r.append("🀐", 1);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("Truncated 2") {
    r.append("🀐", 2);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("Truncated 3") {
    r.append("🀐", 3);
    expected.push_back(0xFFFDU);
  }
  SUBCASE("Truncated 1 valid after") {
    r += "\xF0\x9F\xBC""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("Truncated 2 valid after") {
    r += "\xF0\x9F""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }
  SUBCASE("Truncated 3 valid after") {
    r += "\xF0""A";
    expected.push_back(0xFFFDU);
    expected.push_back('A');
  }

  u32_t i = 0;
  for (auto codepoint : r) {
    REQUIRE(expected.at(i++) == codepoint);
  }
  REQUIRE(i == expected.size());
}

TEST_SUITE_END();
