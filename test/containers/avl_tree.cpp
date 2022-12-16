#include <detail/preamble.hpp>

#include "containers/trees/avl_tree.hpp"
#include "containers/trees/avl_tree_debug.hpp"

namespace t = j::containers::trees;

namespace {
  template<u32_t N>
  void require_tree_values(const t::avl_tree<int, int> & tree, const j::pair<int, int> (& pairs)[N]) {
    J_ASSERT_AVL(tree);
    REQUIRE_UNARY(!tree.empty());
    auto it = tree.begin();
    for (u32_t i = 0; i < N; ++i, ++it) {
      REQUIRE_UNARY(it != tree.end());
      REQUIRE(it->key == pairs[i].first);
      REQUIRE(it->value == pairs[i].second);
      REQUIRE_UNARY(tree.find(pairs[i].first) == it);
      auto it2 = it;
      --it2;
      if (i) {
        REQUIRE_UNARY(it2 != tree.end());
        REQUIRE_UNARY(it2 != tree.before_begin());
        REQUIRE(it2->key == pairs[i - 1].first);
      } else {
        REQUIRE_UNARY(it2 == tree.before_begin());
        REQUIRE_UNARY(it2 != tree.end());
      }
      if (i == N - 1) {
        REQUIRE_UNARY(tree.before_end() == it);
      }
    }
    REQUIRE_UNARY(it == tree.end());
    REQUIRE(tree.front().key == pairs[0].first);
    REQUIRE(tree.back().key == pairs[N - 1].first);
  }
}

TEST_SUITE_BEGIN("Containers - AVL Tree");

TEST_CASE("Empty") {
  t::avl_tree<int, int> tree;
  REQUIRE(tree.empty());
  REQUIRE(tree.begin() == tree.end());
  REQUIRE(tree.find(0) == tree.end());
}

TEST_CASE("Single") {
  t::avl_tree<int, int> tree;
  tree.insert(1,2);
  REQUIRE_UNARY(tree.find(0) == tree.end());
  REQUIRE_UNARY(tree.find(2) == tree.end());
  require_tree_values(tree, {{1, 2}});
}

TEST_CASE("Insert after") {
  t::avl_tree<int, int> tree;
  tree.insert(1,2);
  tree.insert(2,3);
  REQUIRE_UNARY(tree.find(0) == tree.end());
  REQUIRE_UNARY(tree.find(3) == tree.end());
  require_tree_values(tree, {{1,2}, {2,3}});
}

TEST_CASE("Insert before") {
  t::avl_tree<int, int> tree;
  tree.insert(2,3);
  tree.insert(1,2);
  REQUIRE_UNARY(tree.find(0) == tree.end());
  REQUIRE_UNARY(tree.find(3) == tree.end());
  require_tree_values(tree, {{1,2}, {2,3}});
}

TEST_CASE("Insert many") {
  t::avl_tree<int, int> tree;
  const j::pair<int,int> pairs[]{
    {-120,2},
    {-1,77},
    {0,2},
    {1,2},
    {2,3},
    {5,5},
    {9,7},
    {20,2},
  };
  for (int i : (int[]){3, 1, 6, 7, 2, 0, 4, 5}) {
    tree.insert(pairs[i].first, pairs[i].second);
  }
  require_tree_values(tree, pairs);
  REQUIRE(tree.depth() == 4);
}
TEST_CASE("Insert RL") {
  // Trigger right-left rotation
  t::avl_tree<int, int> tree;
  tree.insert(0, 0);
  tree.insert(2, 1);
  // 0              0
  //  \  =INSERT=>   \.
  //   2              2
  //                 /
  //                1
  // 2 becomes left-heavy with a right-heavy parent -> have to do a right-left rotate on the root.
  tree.insert(1, 2);
  require_tree_values(tree, {{0, 0}, {1, 2}, {2, 1}});
  REQUIRE(tree.depth() == 2);
}

TEST_CASE("Insert LR") {
  // Trigger left-right rotation
  t::avl_tree<int, int> tree;
  tree.insert(0, 0);
  tree.insert(-2, 1);
  //    0              0
  //   /  =INSERT=>   /.
  // -2             -2
  //                  \
  //                   -1
  // -2 becomes right-heavy with a left-heavy parent -> have to do a left-right rotate on the root.
  tree.insert(-1, 2);
  require_tree_values(tree, {{-2, 1}, {-1, 2}, {0, 0}});
  REQUIRE(tree.depth() == 2);
}

TEST_CASE("Check depth") {
  t::avl_tree<int, int> tree;
  for (int i = 1; i < 10; ++i) {
    tree.insert(i, i);
  }
  require_tree_values(tree, {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}, {7,7}, {8,8}, {9,9}});
  REQUIRE(tree.depth() == 4);
}

TEST_CASE("Check depth (reverse insert)") {
  t::avl_tree<int, int> tree;
  for (int i = 9; i > 0; --i) {
    tree.insert(i, i);
  }
  require_tree_values(tree, {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}, {7,7}, {8,8}, {9,9}});
  REQUIRE(tree.depth() == 4);
}

TEST_CASE("Lower bound") {
  t::avl_tree<int, int> tree;
  tree.insert(0, 0);
  tree.insert(1, 1);
  tree.insert(7, 7);
  tree.insert(2, 2);
  tree.insert(-10, -10);
  require_tree_values(tree, {{-10,-10}, {0,0}, {1,1}, {2,2}, {7,7}});
  for (auto i : (int[]){-10, 0, 1, 2, 7}) {
    REQUIRE_UNARY(tree.lower_bound(i) == tree.find(i));
  }
  REQUIRE_UNARY(tree.lower_bound(-11) == tree.end());
  REQUIRE_UNARY(tree.lower_bound(-9) == tree.find(-10));
  REQUIRE_UNARY(tree.lower_bound(-1) == tree.find(-10));
  REQUIRE_UNARY(tree.lower_bound(3) == tree.find(2));
  REQUIRE_UNARY(tree.lower_bound(6) == tree.find(2));
  REQUIRE_UNARY(tree.lower_bound(8) == tree.find(7));
}

TEST_CASE("Erase") {
  t::avl_tree<int, int> tree;
  for (int i = 1; i < 10; ++i) {
    tree.insert(i, 0);
    J_ASSERT_AVL(tree);
  }
  int erase_order[] = {1, 7, 8, 2, 5, 6, 9, 4, 3};
  for (int j = 0; j < 9; ++j) {
    REQUIRE_UNARY(tree.find(erase_order[j]) != tree.end());
    tree.erase(erase_order[j]);
    J_ASSERT_AVL(tree);
    REQUIRE_UNARY(tree.find(erase_order[j]) == tree.end());
    auto it = tree.begin();
    for (int i = 1; i < 10; ++i) {
      bool did_find = false;
      for (int k = 0; k <= j; ++k) {
        if (i == erase_order[k]) {
          did_find = true;
        }
      }
      if (did_find) {
        continue;
      }
      REQUIRE_UNARY(it != tree.end());
      REQUIRE(it->key == i);
      ++it;
    }
    REQUIRE_UNARY(it == tree.end());
  }
  REQUIRE_UNARY(tree.empty());
}

TEST_CASE("Erase root") {
  t::avl_tree<int, int> tree;
  tree.insert(1,1);
  tree.insert(0,0);
  tree.insert(2,2);
  require_tree_values(tree, {{0,0},{1,1},{2,2}});
  tree.erase(1);
  require_tree_values(tree, {{0,0},{2,2}});
}

TEST_CASE("Erase root by iterator") {
  t::avl_tree<int, int> tree;
  tree.insert(1,1);
  tree.insert(0,0);
  tree.insert(2,2);
  require_tree_values(tree, {{0,0},{1,1},{2,2}});
  const auto it = tree.erase(tree.find(1));
  require_tree_values(tree, {{0,0},{2,2}});
  REQUIRE_UNARY(it != tree.end());
  REQUIRE(it->key == 2);
}

TEST_CASE("Erase only item by iterator") {
  t::avl_tree<int, int> tree;
  tree.insert(0, 0);
  REQUIRE_UNARY(tree.erase(tree.find(0)) == tree.end());
  REQUIRE_UNARY(tree.empty());
}

TEST_CASE("Erase left item by iterator") {
  t::avl_tree<int, int> tree;
  tree.insert(0, 0);
  tree.insert(-1, 0);
  REQUIRE_UNARY(tree.erase(tree.find(-1)) == tree.begin());
  require_tree_values(tree, {{0, 0}});
}

TEST_CASE("Erase right item by iterator") {
  t::avl_tree<int, int> tree;
  tree.insert(0, 0);
  tree.insert(1, 0);
  REQUIRE_UNARY(tree.erase(tree.find(1)) == tree.end());
  require_tree_values(tree, {{0, 0}});
}

TEST_CASE("Erase by iterator") {
  t::avl_tree<int, int> tree;
  for (int i = 1; i < 10; ++i) {
    tree.insert(i, i);
  }
  auto it = tree.begin();
  for (int i = 1; i < 10; ++i) {
    REQUIRE_UNARY(it != tree.end());
    REQUIRE(it->key == i);
    it = tree.erase(it);
  }
  REQUIRE_UNARY(it == tree.end());
}

TEST_SUITE_END();
