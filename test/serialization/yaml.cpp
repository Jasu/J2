#include <detail/preamble.hpp>

#include "serialization/yaml/yaml_backend.hpp"
#include "detail/temp_file.hpp"
#include "strings/string_view.hpp"
#include "properties/access.hpp"
#include "containers/pair.hpp"
#include "files/paths/path.hpp"
#include "properties/wrappers/stored_wrapper.hpp"

TEST_SUITE_BEGIN("Serialization - YAML");

namespace {
  namespace p = j::properties;

  using pair = j::pair<const char *, const char *>;

  j::serialization::yaml::yaml_backend b;

  p::access::map_access assert_map(p::wrappers::wrapper & w, u32_t sz) {
    REQUIRE(w.is_map());
    auto m{w.as_map()};
    REQUIRE(m.size() == sz);
    REQUIRE(m.empty() == (sz == 0));
    return m;
  }

  template<u32_t N>
  void assert_string_map_contents(p::wrappers::wrapper & w, const pair(& pairs)[N]) {
    auto m = assert_map(w, N);
    auto it = m.begin();
    for (u32_t i = 0; i < N; ++i, ++it) {
      REQUIRE_UNARY(it != m.end());
      REQUIRE_UNARY(it->key().is_string());
      REQUIRE_EQ(it->key().as_string().as_j_string_view(), pairs[i].first);
      REQUIRE_UNARY(it->value().is_string());
      REQUIRE_EQ(it->value().as_string().as_j_string_view(), pairs[i].second);
    }
    REQUIRE(it == m.end());
  }

  p::access::list_access assert_list(p::wrappers::wrapper & w, u32_t sz) {
    REQUIRE_UNARY(w.is_list());
    auto l{w.as_list()};
    REQUIRE_EQ(l.size(), sz);
    REQUIRE_EQ(l.empty(), (sz == 0));
    return l;
  }

  template<u32_t N>
  void assert_string_list_contents(p::wrappers::wrapper & w, const char * const(& strings)[N]) {
    auto l = assert_list(w, N);
    auto it = l.begin();
    for (u32_t i = 0; i < N; ++i, ++it) {
      REQUIRE_UNARY(it != l.end());
      REQUIRE_UNARY(it->is_string());
      REQUIRE_EQ(it->as_string().as_j_string_view(), strings[i]);
      REQUIRE_UNARY(l.at(i).is_string());
      REQUIRE_EQ(l.at(i).as_string().as_j_string_view(), strings[i]);
    }
    REQUIRE_UNARY(it == l.end());
  }

  template<u32_t N>
  void assert_int_list_contents(p::wrappers::wrapper & w, i64_t const(& ints)[N]) {
    auto l = assert_list(w, N);
    auto it = l.begin();
    for (u32_t i = 0; i < N; ++i, ++it) {
      REQUIRE_UNARY(it != l.end());
      REQUIRE_UNARY(it->is_int());
      REQUIRE_EQ(it->as_int().get_value(), ints[i]);
      REQUIRE_UNARY(l.at(i).is_int());
      REQUIRE_EQ(l.at(i).as_int().get_value(), ints[i]);
    }
    REQUIRE_UNARY(it == l.end());
  }

  template<u32_t N>
  void assert_float_list_contents(p::wrappers::wrapper & w, double const(& doubles)[N]) {
    auto l = assert_list(w, N);
    auto it = l.begin();
    for (u32_t i = 0; i < N; ++i, ++it) {
      REQUIRE_UNARY(it != l.end());
      REQUIRE_UNARY(it->is_float());
      REQUIRE_EQ(it->as_float().get_value(), doubles[i]);
      REQUIRE_UNARY(l.at(i).is_float());
      REQUIRE_EQ(l.at(i).as_float().get_value(), doubles[i]);
    }
    REQUIRE(it == l.end());
  }

  template<u32_t N>
  void assert_bool_list_contents(p::wrappers::wrapper & w, i64_t const(& bools)[N]) {
    auto l = assert_list(w, N);
    auto it = l.begin();
    for (u32_t i = 0; i < N; ++i, ++it) {
      REQUIRE_UNARY(it != l.end());
      REQUIRE_UNARY(it->is_bool());
      REQUIRE_EQ(it->as_bool().get_value(), bools[i]);
      REQUIRE_UNARY(l.at(0).is_bool());
      REQUIRE_EQ(l.at(0).as_bool().get_value(), bools[i]);
    }
    REQUIRE_UNARY(it == l.end());
  }
}

TEST_CASE("Empty map") {
  ::detail::temp_file file("{}");
  auto resource = b.read(file.path());
  REQUIRE_UNARY(resource.wrapper().is_map());
  auto m = resource.wrapper().as_map();
  REQUIRE_EQ(m.size(), 0);
  REQUIRE_UNARY(m.empty());
  REQUIRE_UNARY(m.begin() == m.end());
}

TEST_CASE("Map with a single string value") {
  ::detail::temp_file file("{hello: world}");
  auto resource = b.read(file.path());
  assert_string_map_contents(resource.wrapper(), {{"hello", "world"}});
}

TEST_CASE("Map with two string values") {
  ::detail::temp_file file("hello: world\n"
                           "ohno: \"goodbye.\"\n");
  auto resource = b.read(file.path());
  assert_string_map_contents(resource.wrapper(), {{"hello", "world"}, {"ohno", "goodbye."}});
}

TEST_CASE("Empty list") {
  ::detail::temp_file file("[]");
  auto resource = b.read(file.path());
  assert_list(resource.wrapper(), 0);
}

TEST_CASE("List with a single string") {
  ::detail::temp_file file("- hello\n");
  auto resource = b.read(file.path());
  assert_string_list_contents(resource.wrapper(), {"hello"});
}

TEST_CASE("List with a single int") {
  ::detail::temp_file file("- 1\n");
  auto resource = b.read(file.path());
  assert_int_list_contents(resource.wrapper(), {1});
}

TEST_CASE("List with a single float") {
  ::detail::temp_file file("- 12.3\n");
  auto resource = b.read(file.path());
  assert_float_list_contents(resource.wrapper(), {12.3});
}

TEST_CASE("List with a single true") {
  ::detail::temp_file file("- true\n");
  auto resource = b.read(file.path());
  assert_bool_list_contents(resource.wrapper(), {true});
}

TEST_CASE("List with a single false") {
  ::detail::temp_file file("- false\n");
  auto resource = b.read(file.path());
  assert_bool_list_contents(resource.wrapper(), {false});
}

TEST_CASE("List with a single null") {
  ::detail::temp_file file("- null\n");
  auto resource = b.read(file.path());
  auto l = assert_list(resource.wrapper(), 1);
  REQUIRE_UNARY(l.at(0).is_nil());
}

TEST_SUITE_END();
