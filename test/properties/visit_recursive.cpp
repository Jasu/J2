#include <detail/preamble.hpp>

#include "properties/access.hpp"
#include "properties/visiting/visit_recursive.hpp"
#include "properties/property_variant.hpp"
#include "properties/classes/object_access_registration.hpp"
#include "containers/unsorted_string_map.hpp"
#include "containers/vector.hpp"

using namespace j::properties;
using namespace j::properties::access;
using namespace j::properties::visiting;
using namespace j::properties::classes;

namespace {
  void visit_once_impl(typed_access & access, const visit_path & p, PROPERTY_TYPE property_type) {
    REQUIRE_UNARY(p.type() == COMPONENT_TYPE::EMPTY);
    REQUIRE(p.is_root());
    REQUIRE_FALSE(p.is_map_key());
    REQUIRE_FALSE(p.is_property_name());
    REQUIRE_FALSE(p.is_array_index());
    REQUIRE(p.parent() == nullptr);
    REQUIRE(((path)p).empty());
    REQUIRE_UNARY(access.type() == property_type);
  }

  void assert_path_array_index(const path & p, u32_t sz, sz_t array_index) {
    REQUIRE(p.size() == sz);
    REQUIRE_UNARY(!p.empty());
    REQUIRE(p.is_array_index());
    REQUIRE(p[sz - 1].is_array_index());
    REQUIRE(p[sz - 1].as_array_index() == array_index);
  }

  void visit_once(typed_access & a, const std::type_info & access_type, PROPERTY_TYPE property_type) {
    u32_t num_accesses = 0;
    visit_recursive(a, [&](auto & access, const visit_path & p) -> void {
      REQUIRE_UNARY(typeid(j::decay_t<decltype(access)>).hash_code() == access_type.hash_code());
      ++num_accesses;
      visit_once_impl(access, p, property_type);
    });
    REQUIRE_UNARY(num_accesses == 1);
  }

  template<typename Access>
  j::vector<path> collect_paths(Access && a) {
    j::vector<path> result;
    visit_recursive(static_cast<Access &&>(a), [&result](typed_access &, const visit_path & p) -> void {
      result.emplace_back(p);
    });
    REQUIRE(!result.empty());
    REQUIRE(result[0].is_root());
    REQUIRE(result[0].size() == 0);
    return result;
  }
}


TEST_CASE("Visit recursive nil") {
  auto nil = nullptr;
  typed_access a{nil};
  visit_once(a, typeid(nil_access), PROPERTY_TYPE::NIL);
}

TEST_CASE("Visit recursive string") {
  j::strings::string s = "foo";
  typed_access a{s};
  visit_once(a, typeid(string_access), PROPERTY_TYPE::STRING);
}

TEST_CASE("Visit recursive int") {
  int i = 9999;
  typed_access a{i};
  visit_once(a, typeid(int_access), PROPERTY_TYPE::INT);
}

TEST_CASE("Visit recursive bool") {
  bool b = false;
  typed_access a{b};
  visit_once(a, typeid(bool_access), PROPERTY_TYPE::BOOL);
}

TEST_CASE("Visit recursive float") {
  float f = 1.999f;
  typed_access a{f};
  visit_once(a, typeid(float_access), PROPERTY_TYPE::FLOAT);
}

TEST_CASE("Visit recursive empty list") {
  list_t l;
  typed_access a{l};
  visit_once(a, typeid(list_access), PROPERTY_TYPE::LIST);
}

TEST_CASE("Visit recursive empty map") {
  map_t m;
  typed_access a{m};
  visit_once(a, typeid(map_access), PROPERTY_TYPE::MAP);
}

namespace {
  struct empty_object {};
  object_access_registration<empty_object> empty_reg{"empty_object"};
}

TEST_CASE("Visit recursive empty object") {
  empty_object o;
  typed_access a{o};
  visit_once(a, typeid(object_access), PROPERTY_TYPE::OBJECT);
}

namespace {
  struct simple_object {
    int x = 0;
    float y = 1;
  };
  object_access_registration<simple_object> simple_reg{
    "simple_object",
    property = member<&simple_object::x>("x"),
    property = member<&simple_object::y>("y")
  };
}

TEST_CASE("Visit recursive simple object") {
  simple_object o;
  typed_access a{o};
  auto paths = collect_paths(a);
  REQUIRE(paths.size() == 3);
  REQUIRE(paths[0].is_root());
  REQUIRE(paths[0].size() == 0);

  REQUIRE(paths[1].size() == 1);
  REQUIRE(paths[1].is_property_name());
  REQUIRE(paths[1][0].is_property_name());

  REQUIRE(paths[2].size() == 1);
  REQUIRE(paths[2].is_property_name());
  REQUIRE(paths[2][0].is_property_name());

  j::strings::string n1 = paths[1][0].as_property_name();
  j::strings::string n2 = paths[2][0].as_property_name();
  REQUIRE(((n1 == "x" && n2 == "y") || (n1 == "y" && n2 == "x")));
}

TEST_CASE("Visit recursive map") {
  map_t m;
  m.insert("key1", "foo");
  m.insert("key2", "foo");
  typed_access a{m};
  auto paths = collect_paths(a);
  REQUIRE(paths.size() == 3);
  REQUIRE(paths[0].size() == 0);
  REQUIRE(paths[0].is_root());

  REQUIRE(paths[1].size() == 1);
  REQUIRE(paths[1].is_map_key());
  REQUIRE(paths[1][0].is_map_key());

  REQUIRE(paths[2].size() == 1);
  REQUIRE(paths[2].is_map_key());
  REQUIRE(paths[2][0].is_map_key());

  j::strings::string n1 = paths[1][0].as_map_key();
  j::strings::string n2 = paths[2][0].as_map_key();
  REQUIRE(((n1 == "key2" && n2 == "key1") || (n1 == "key1" && n2 == "key2")));
}

TEST_CASE("Visit recursive list") {
  list_t l;
  l.emplace_back("1");
  l.emplace_back("2");
  typed_access a{l};
  auto paths = collect_paths(a);
  REQUIRE(paths.size() == 3);

  assert_path_array_index(paths[1], 1, 0);
  assert_path_array_index(paths[2], 1, 1);
}

namespace {
  struct nested_object {
    int x;
    list_t l;
    int y;
    map_t m;
  };
  object_access_registration<nested_object> nested_reg{
    "nested_object",
    property = member<&nested_object::x>("x"),
    property = member<&nested_object::l>("l"),
    property = member<&nested_object::y>("y"),
    property = member<&nested_object::m>("m")
  };
}

TEST_CASE("Visit recursive nested object") {
  nested_object o;
  o.x = 1;
  o.y = 2;
  o.l.emplace_back(1);
  o.l.emplace_back(2);
  o.l.emplace_back(3);
  o.m.insert("hello", 7);
  o.m.insert("world", 7);

  typed_access a{o};
  auto paths = collect_paths(a);
  REQUIRE(paths.size() == 10);

  bool has_x = false, has_l = false, has_y = false, has_m = false;
  for (auto it = paths.begin() + 1; it != paths.end(); ++it) {
    REQUIRE(it->size() == 1);
    REQUIRE(it->is_property_name());
    REQUIRE((*it)[0].is_property_name());
    if ((*it)[0].as_property_name() == "x") {
      has_x = true;
    } else if ((*it)[0].as_property_name() == "y") {
      has_y = true;
    } else if ((*it)[0].as_property_name() == "l") {
      has_l = true;
      for (int i = 0; i < 3; ++i) {
        ++it;
        assert_path_array_index(*it, 2, i);
        REQUIRE((*it)[0].is_property_name());
        REQUIRE_UNARY((*it)[0].as_property_name() == "l");
      }
    } else if ((*it)[0].as_property_name() == "m") {
      has_m = true;
      for (int i = 0; i < 2; ++i) {
        ++it;
        REQUIRE(it->size() == 2);
        REQUIRE(it->is_map_key());
        REQUIRE((*it)[0].is_property_name());
        REQUIRE_UNARY((*it)[0].as_property_name() == "m");
        REQUIRE((*it)[1].is_map_key());
      }
      REQUIRE(((*it)[1].as_map_key() == "hello" && (*(it - 1))[1].as_map_key() == "world"
               || (*it)[1].as_map_key() == "world" && (*(it - 1))[1].as_map_key() == "hello"));
    } else {
      FAIL("Unexpected key.");
    }
  }
  REQUIRE((has_x && has_y && has_l && has_m));
}

TEST_CASE("Visit recursive nested list") {
  list_t sublist;
  sublist.emplace_back("1");
  sublist.emplace_back("1");
  list_t l;
  l.emplace_back("1");
  l.emplace_back("2");
  l.emplace_back(sublist);
  l.emplace_back(sublist);
  l.emplace_back("2");
  typed_access a{l};
  auto paths = collect_paths(a);
  REQUIRE(paths.size() == 10);

  assert_path_array_index(paths[1], 1, 0);
  assert_path_array_index(paths[2], 1, 1);
  assert_path_array_index(paths[3], 1, 2);

  assert_path_array_index(paths[4], 2, 0);
  REQUIRE(paths[4][0].is_array_index());
  REQUIRE(paths[4][0].as_array_index() == 2);

  assert_path_array_index(paths[5], 2, 1);
  REQUIRE(paths[5][0].is_array_index());
  REQUIRE(paths[5][0].as_array_index() == 2);

  assert_path_array_index(paths[6], 1, 3);

  assert_path_array_index(paths[7], 2, 0);
  REQUIRE(paths[7][0].is_array_index());
  REQUIRE(paths[7][0].as_array_index() == 3);

  assert_path_array_index(paths[8], 2, 1);
  REQUIRE(paths[8][0].is_array_index());
  REQUIRE(paths[8][0].as_array_index() == 3);

  assert_path_array_index(paths[9], 1, 4);
}

TEST_CASE("Visit recursive nested map") {
  list_t sublist;
  sublist.emplace_back(1);
  sublist.emplace_back(2);

  map_t m;
  m.insert("key1", "foo");
  m.insert("key2", "foo");
  m.insert("key3", sublist);
  m.insert("key4", sublist);
  typed_access a{m};

  auto paths = collect_paths(a);
  REQUIRE(paths.size() == 9);

  for (auto it = paths.begin() + 1; it != paths.end(); ++it) {
    REQUIRE(it->size() == 1);
    REQUIRE(it->is_map_key());
    REQUIRE((*it)[0].is_map_key());
    if ((*it)[0].as_map_key() == "key1") {
    } else if ((*it)[0].as_map_key() == "key2") {
    } else if ((*it)[0].as_map_key() == "key3") {
      for (int i = 0; i < 2; ++i) {
        ++it;
        REQUIRE(it->size() == 2);
        REQUIRE(it->is_array_index());
        REQUIRE((*it)[0].is_map_key());
        REQUIRE_UNARY((*it)[0].as_map_key() == "key3");
        REQUIRE((*it)[1].is_array_index());
        REQUIRE((*it)[1].as_array_index() == i);
      }
    } else if ((*it)[0].as_map_key() == "key4") {
      for (int i = 0; i < 2; ++i) {
        ++it;
        REQUIRE(it->size() == 2);
        REQUIRE(it->is_array_index());
        REQUIRE((*it)[0].is_map_key());
        REQUIRE_UNARY((*it)[0].as_map_key() == "key4");
        REQUIRE((*it)[1].is_array_index());
        REQUIRE((*it)[1].as_array_index() == i);
      }
    } else {
      FAIL("Unexpected key.");
    }
  }
}
