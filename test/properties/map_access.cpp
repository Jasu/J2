#include <detail/preamble.hpp>

#include "properties/access/map_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/property_variant.hpp"
#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/wrappers/variant_wrapper.hpp"
#include "containers/unsorted_string_map.hpp"

TEST_CASE("map_t access read") {
  j::properties::map_t l{};
  l.emplace("k1", j::properties::property_variant("hello"));
  l.emplace("k2", j::properties::property_variant(1));
  j::properties::access::map_access a{l};
  REQUIRE(!a.empty());
  REQUIRE(a.size() == 2);
  REQUIRE(a.at("k1").type() == j::properties::PROPERTY_TYPE::STRING);
  REQUIRE(a.at("k2").type() == j::properties::PROPERTY_TYPE::INT);
}

TEST_CASE("map_t access clear") {
  j::properties::map_t l{};
  l.emplace("k1", j::properties::property_variant("hello"));
  l.emplace("k2", j::properties::property_variant(1));
  j::properties::access::map_access a{l};
  a.clear();
  REQUIRE(l.empty());
  REQUIRE(l.size() == 0);
}

TEST_CASE("property_variant map_t access read") {
  j::properties::map_t l{};
  l.emplace("k1", j::properties::property_variant("hello"));
  l.emplace("k2", j::properties::property_variant(1));
  j::properties::property_variant v{l};
  auto w = j::properties::wrappers::access(v);

  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::MAP);
  REQUIRE(w.is_map());
  REQUIRE(!w.as_map().empty());
  REQUIRE(w.as_map().size() == 2);
  REQUIRE(w.as_map().at("k1").type() == j::properties::PROPERTY_TYPE::STRING);
  REQUIRE(w.as_map().at("k2").type() == j::properties::PROPERTY_TYPE::INT);
}

TEST_CASE("property_variant map_t access clear") {
  j::properties::map_t l{};
  l.emplace("k1", j::properties::property_variant("hello"));
  l.emplace("k2", j::properties::property_variant(1));
  j::properties::property_variant v{l};
  auto w = j::properties::wrappers::access(v);
  REQUIRE(w.type() == j::properties::PROPERTY_TYPE::MAP);
  REQUIRE(w.is_map());
  w.as_map().clear();
  REQUIRE(v.as<j::properties::map_t>().empty());
  REQUIRE(v.as<j::properties::map_t>().size() == 0);
}


TEST_CASE("map_t access iteration") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  auto it = m.begin();
  for (auto & p : a) {
    REQUIRE(p.key().as_string().as_j_string() == *it->first);
    REQUIRE(p.value().type() == it->second->type());
    ++it;
  }
}

TEST_CASE("map_t access find") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  auto it = a.find("frooba");
  REQUIRE(it == a.end());
  it = a.find("k1");
  REQUIRE(it != a.end());
  REQUIRE(it->key().is_string());
  REQUIRE(it->key().as_string().as_j_string() == "k1");
  REQUIRE(it->value().is_string());
  REQUIRE(it->value().as_string().as_j_string() == "hello");
}

TEST_CASE("map_t access erase") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  REQUIRE(a.erase("k2") == 1);
  REQUIRE(m.size() == 2);
  REQUIRE(m.find("k2") == m.end());
  a.erase("key");
  REQUIRE(m.size() == 1);
  REQUIRE(m.find("key") == m.end());
}

TEST_CASE("map_t access erase fully") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  sz_t expected_sz = 3;
  for (auto it = a.begin(); it != a.end();) {
    it = a.erase(it);
    --expected_sz;
    REQUIRE(m.size() == expected_sz);
  }
  REQUIRE(m.size() == 0);
  REQUIRE(expected_sz == 0);
}

TEST_CASE("map_t insert int") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  auto result = a.insert("k3", 1839);
  REQUIRE(m.size() == 4);
  REQUIRE(m.find("k3") != m.end());
  REQUIRE(m.at("k3").is_int());
  REQUIRE(m.at("k3").as_int() == 1839);
  REQUIRE(result.second);
  REQUIRE(result.first != a.end());
  REQUIRE(result.first->key().is_string());
  REQUIRE(result.first->key().as_string().as_j_string() == "k3");
  REQUIRE(result.first->value().is_int());

  // Inserting does not replace
  REQUIRE(result.first->value().as_int().get_value() == 1839);
  result = a.insert("k3", false);
  REQUIRE(m.size() == 4);
  REQUIRE(m.find("k3") != m.end());
  REQUIRE(m.at("k3").is_int());
  REQUIRE(m.at("k3").as_int() == 1839);
}

TEST_CASE("map_t insert with conversion") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  auto result = a.insert("k3", "test");
  REQUIRE(m.size() == 4);
  REQUIRE(m.find("k3") != m.end());
  REQUIRE(m.at("k3").is_string());
  REQUIRE(m.at("k3").as_string() == "test");
  REQUIRE(result.second);
  REQUIRE(result.first != a.end());
  REQUIRE(result.first->key().is_string());
  REQUIRE(result.first->key().as_string().as_j_string() == "k3");
  REQUIRE(result.first->value().is_string());
  REQUIRE(result.first->value().as_string().as_j_string() == "test");
}

TEST_CASE("map_t replace") {
  j::properties::map_t m{};
  m.emplace("k1", "hello");
  m.emplace("k2", 1);
  m.emplace("key", false);
  j::properties::access::map_access a{m};
  auto result = a.insert("k2", "test");
  REQUIRE(m.size() == 3);
  REQUIRE(m.find("k2") != m.end());
  REQUIRE(m.at("k2").is_int());
  REQUIRE(result.first != a.end());
  REQUIRE(result.first == a.find("k2"));
  REQUIRE(!result.second);

  auto result2 = a.replace("k2", "test");
  REQUIRE(m.size() == 3);
  REQUIRE(m.find("k2") != m.end());
  REQUIRE(m.at("k2").is_string());
  REQUIRE(m.at("k2").as_string() == "test");
  REQUIRE(result2 == a.find("k2"));
}
