#include <detail/preamble.hpp>

#include "containers/hash_map.hpp"

TEST_SUITE_BEGIN("Containers - Hash Map");

namespace {
  using map_t = j::hash_map<u32_t, u64_t>;
  using pair_t = j::pair<u32_t, u64_t>;

  template<typename Map, typename K, typename V>
  void checked_insert(Map & map, K && k, V && v) {
    auto key = k;
    auto value = v;
    REQUIRE_UNARY(!map.contains(key));
    REQUIRE_UNARY(map.find(key) == map.end());
    auto p = map.insert(static_cast<K &&>(k), static_cast<V &&>(v));
    REQUIRE_UNARY(p.second);
    REQUIRE_UNARY(p.first != map.end());
    REQUIRE_UNARY(p.first == map.find(key));
    REQUIRE_UNARY(map.contains(key));
    REQUIRE_UNARY((i64_t)p.first->first == (i64_t)key);
    REQUIRE_UNARY((i64_t)p.first->second == (i64_t)value);
  }

  template<typename Map, typename K, typename V>
  void checked_insert_fail(Map & map, K && k, V && v) {
    auto key = k;
    auto p = map.insert(static_cast<K &&>(k), static_cast<V &&>(v));
    REQUIRE_UNARY(!p.second);
    REQUIRE_UNARY(p.first != map.end());
    REQUIRE_UNARY(p.first == map.find(key));
    REQUIRE_UNARY(p.first->first == key);
  }

  void assert_map_contents(const map_t & map, const pair_t * pairs, i32_t sz) {
    REQUIRE_UNARY(map.empty() == (sz == 0));
    REQUIRE_UNARY(map.size() == sz);
    for (i32_t i = 0; i < sz; ++i) {
      REQUIRE_UNARY(map.at(pairs[i].first) == pairs[i].second);
    }
    auto it = map.begin(), end = map.end();
    bool met_indices[sz];
    ::j::memset(met_indices, 0, sizeof(met_indices));
    for (i32_t i = 0; i < sz; ++i, ++it) {
      REQUIRE_UNARY(it != end);
      REQUIRE_UNARY(it == map.find(it->first));
      REQUIRE_UNARY(map.contains(it->first));
      for (i32_t j = 0; j < sz; ++j) {
        if (pairs[j].first == it->first) {
          REQUIRE_UNARY(pairs[j].second == it->second);
          met_indices[j] = true;
          goto did_find;
        }
      }
      FAIL("Did not find.");
    did_find: ;
    }
    for (i32_t i = 0; i < sz; ++i) {
      REQUIRE_UNARY(met_indices[i]);
    }
    REQUIRE_UNARY(it == end);
  }

  template<i32_t N>
  void assert_map_contents(const map_t & map, const pair_t (& pairs)[N]) {
    assert_map_contents(map, pairs, N);
  }
}

TEST_CASE("Empty") {
  map_t map;
  REQUIRE_UNARY(map.empty());
  REQUIRE(map.size() == 0U);
}

TEST_CASE("Single") {
  map_t map;
  checked_insert(map, 1, 2);
  assert_map_contents(map, {{1, 2}});
}

TEST_CASE("Multiple") {
  map_t map;
  pair_t pairs[64];
  for (u32_t i = 0U; i < 64U; ++i) {
    checked_insert(map, i, i * 2);
    pairs[i] = {i, i * 2};
    assert_map_contents(map, pairs, i + 1U);
  }
}

TEST_CASE("No overwrite") {
  map_t map;
  pair_t pairs[64];
  for (u32_t i = 0U; i < 64U; ++i) {
    checked_insert(map, i, i * 2);
    for (u32_t j = 0U; j <= i; ++j) {
      checked_insert_fail(map, i, i * 3 + 1);
    }
    pairs[i] = {i, i * 2};
    assert_map_contents(map, pairs, i + 1U);
  }
}

TEST_CASE("Erase") {
  map_t map;
  pair_t pairs[64];
  for (u32_t i = 0U; i < 64U; ++i) {
    map.insert(i, i * 2);
    pairs[i] = {i, i * 2};
  }

  for (i32_t i = 0U; i < 64; ++i) {
    map.erase(map.find(i));
    REQUIRE_UNARY(!map.contains(i));
    REQUIRE_UNARY(map.find(i) == map.end());
    assert_map_contents(map, pairs + i + 1U, 63U - i);
  }
}

TEST_CASE("Erase full") {
  map_t map;
  for (i32_t i = 0U; i < 64; ++i) {
    map.insert(i, i * 2);
  }

  i32_t sz = 64;
  for (auto it = map.begin(); it != map.end(); --sz) {
    auto k = it->first;
    REQUIRE_UNARY(map.contains(k));
    it = map.erase(it);
    REQUIRE_UNARY(map.find(k) == map.end());
    REQUIRE_UNARY(!map.contains(k));
    REQUIRE(map.size() == sz - 1U);
  }
}

TEST_SUITE_END();
