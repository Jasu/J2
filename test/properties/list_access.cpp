#include <detail/preamble.hpp>

#include "properties/access/list_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/property_variant.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/wrappers/variant_wrapper.hpp"
#include "containers/vector.hpp"

TEST_CASE("list_t access read") {
  j::properties::list_t l{};
  l.emplace_back("hello");
  l.emplace_back(1);
  j::properties::access::list_access a{l};
  REQUIRE(!l.empty());
  REQUIRE(l.size() == 2);
  REQUIRE(l.at(0).type() == j::properties::PROPERTY_TYPE::STRING);
  REQUIRE(l.at(0).is_string());
  REQUIRE(l.at(1).type() == j::properties::PROPERTY_TYPE::INT);
  REQUIRE(l.at(1).is_int());
}

TEST_CASE("list_t access clear") {
  j::properties::list_t l{};
  l.emplace_back("hello");
  l.emplace_back(1);
  j::properties::access::list_access a{l};
  a.clear();
  REQUIRE(a.empty());
  REQUIRE(l.empty());
}

TEST_CASE("variant list_t access read") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  REQUIRE(w.as_list().size() == 2);
  REQUIRE(w.as_list().at(0).type() == j::properties::PROPERTY_TYPE::STRING);
  REQUIRE(w.as_list().at(0).is_string());
  REQUIRE(w.as_list().at(1).type() == j::properties::PROPERTY_TYPE::FLOAT);
  REQUIRE(w.as_list().at(1).is_float());
}

TEST_CASE("variant list_t access clear") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  w.as_list().clear();
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  REQUIRE(w.as_list().empty());
  REQUIRE(v.as<j::properties::list_t>().empty());
}

TEST_CASE("variant list_t iteration") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.as_list().begin() == w.as_list().begin());
  REQUIRE(w.as_list().end() == w.as_list().end());
  REQUIRE(w.as_list().begin() != w.as_list().end());
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  sz_t i = 0;
  for (auto item : w.as_list()) {
    switch(i++) {
    case 0:
      REQUIRE(item.is_string());
      REQUIRE(item.as_string().as_j_string() == "hello");
      break;
    case 1:
      REQUIRE(item.is_float());
      REQUIRE(item.as_float().get_value() == 2.0);
      break;
    default:
      FAIL("Unexpected index.");
    }
  }
  REQUIRE(i == 2);
}

TEST_CASE("variant list_t erase by index") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  w.as_list().erase(1);
  REQUIRE(w.as_list().size() == 1);
  REQUIRE(v.as_list().size() == 1);
  REQUIRE(v.as_list().at(0).is_string());
  REQUIRE(v.as_list().at(0).as_string() == "hello");

  v.as<j::properties::list_t>().emplace_back(2.0);
  w.as_list().erase(0);
  REQUIRE(w.as_list().size() == 1);
  REQUIRE(v.as_list().size() == 1);
  REQUIRE(v.as_list().at(0).is_float());
  REQUIRE(v.as_list().at(0).as_float() == 2.0);
}

TEST_CASE("variant list_t erase by iterator") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  auto it = w.as_list().erase(w.as_list().begin());
  REQUIRE(w.as_list().size() == 1);
  REQUIRE(v.as_list().size() == 1);
  REQUIRE(v.as_list().at(0).is_float());
  REQUIRE(v.as_list().at(0).as_float() == 2.0);
  REQUIRE(it == w.as_list().begin());

  v.as<j::properties::list_t>().emplace_back(3.0);
  REQUIRE(w.as_list().size() == 2);
  REQUIRE(v.as_list().size() == 2);
  REQUIRE(v.as_list().at(1).is_float());
  REQUIRE(v.as_list().at(1).as_float() == 3.0);

  it = w.as_list().begin();
  ++it;
  REQUIRE(it != w.as_list().begin());
  REQUIRE(it != w.as_list().end());
  it = w.as_list().erase(it);
  REQUIRE(w.as_list().size() == 1);
  REQUIRE(v.as_list().size() == 1);
  REQUIRE(v.as_list().at(0).is_float());
  REQUIRE(v.as_list().at(0).as_float() == 2.0);
  REQUIRE(it == w.as_list().end());
  REQUIRE(it != w.as_list().begin());
}

TEST_CASE("variant list_t push_back") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  w.as_list().push_back(7.0);
  REQUIRE(w.as_list().size() == 3);
  REQUIRE(v.as_list()[0].as_string() == "hello");
  REQUIRE(v.as_list()[1].as_float() == 2.0);
  REQUIRE(v.as_list()[2].as_float() == 7.0);
}

TEST_CASE("variant list_t insert") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  auto it = w.as_list().begin();
  ++it;
  REQUIRE(it->is_float());
  it = w.as_list().insert(it, 9);
  REQUIRE(it->is_int());
  REQUIRE(v.as_list().size() == 3);
  REQUIRE(v.as_list()[0].is_string());
  REQUIRE(v.as_list()[1].is_int());
  REQUIRE(v.as_list()[2].is_float());
}

TEST_CASE("variant list_t insert at end") {
  j::properties::property_variant v = j::properties::list_t{};
  v.as<j::properties::list_t>().emplace_back("hello");
  v.as<j::properties::list_t>().emplace_back(2.0);
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::LIST);
  REQUIRE(w.is_list());
  auto it = w.as_list().end();
  it = w.as_list().insert(it, 9);
  REQUIRE(it != w.as_list().end());
  REQUIRE(it->is_int());
  REQUIRE(v.as_list().size() == 3);
  REQUIRE(v.as_list()[0].is_string());
  REQUIRE(v.as_list()[1].is_float());
  REQUIRE(v.as_list()[2].is_int());
}
