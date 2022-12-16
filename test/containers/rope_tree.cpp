#include <detail/preamble.hpp>

#include "containers/trees/rope_tree.hpp"
#include "containers/trees/rope_tree_debug.hpp"
#include "containers/vector.hpp"
#include "strings/string_view.hpp"

namespace s = j::strings;
namespace t = j::trees;

TEST_SUITE_BEGIN("Containers - Rope Tree");

namespace {
  struct J_TYPE_HIDDEN rope_part;
  struct J_TYPE_HIDDEN rope_metrics;

  struct J_TYPE_HIDDEN rope_ctrl final {
    rope_metrics compute_metrics(const rope_part & value) const noexcept;
  };

  struct J_TYPE_HIDDEN rope_metrics final {
    i32_t size = 0U;

    [[nodiscard]] explicit operator bool() const noexcept { return size; }

    [[nodiscard]] s::string format() const {
      return s::format("{}", size);
    }

    [[nodiscard]] i32_t key() const noexcept {
      return size;
    }

    [[nodiscard]] bool operator==(const rope_metrics & rhs) const noexcept {
      return size == rhs.size;
    }

    rope_metrics & operator+=(const rope_metrics & rhs) noexcept {
      size += rhs.size;
      return *this;
    }

    rope_metrics & operator-=(const rope_metrics & rhs) noexcept {
      size -= rhs.size;
      return *this;
    }

    [[nodiscard]] rope_metrics operator-() const noexcept {
      return { -size };
    }

    [[nodiscard]] rope_metrics operator+(const rope_metrics & rhs) const noexcept {
      return { size + rhs.size };
    }

    [[nodiscard]] rope_metrics operator-(const rope_metrics & rhs) const noexcept {
      return { size - rhs.size };
    }
  };

  struct J_TYPE_HIDDEN rope_state_diff final {
    u64_t old_value_id = 0;
    u64_t new_value_id = 0;
    i32_t metrics_d = 0;

    rope_state_diff() noexcept = default;

    rope_state_diff(const rope_part * p) noexcept;

    [[nodiscard]] rope_metrics metrics_delta() const noexcept {
      return rope_metrics{metrics_d};
    }
    void reset_metrics_delta() noexcept {
      metrics_d = 0;
    }

    [[nodiscard]] bool should_propagate() const noexcept {
      return old_value_id != new_value_id;
    }
  };

  struct J_TYPE_HIDDEN rope_description final {
    using value_t = rope_part;
    using controller_t = rope_ctrl;
    using key_t = u32_t;
    using key_diff_t = i32_t;
    using state_diff_t = rope_state_diff;
    using metrics_t = rope_metrics;
    static constexpr inline u8_t max_size_v = 4U;
  };




  struct J_TYPE_HIDDEN rope_part final {
    s::const_string_view value;
    t::rope_tree_ref<rope_description> ref;

    inline static u64_t g_value_id = 0U;
    u64_t value_id = 0U;
    u64_t value_id_before = 0U;

    [[nodiscard]] s::string format() const {
      return value;
    }

    constexpr rope_part() noexcept = default;
    rope_part(s::const_string_view value) noexcept
      : value(value),
        value_id(++g_value_id)
    { }

    void initialize(s::const_string_view value) noexcept {
      this->value = value;
      this->value_id = ++g_value_id;
    }

    void initialize_state(rope_state_diff & diff) {
      value_id_before = diff.new_value_id;
      diff.metrics_d = value.size();
      diff.new_value_id = value_id;
    }

    void update_state(rope_state_diff & diff) {
      J_ASSERT(diff.old_value_id == value_id_before, "Value id before mismatch");
      value_id_before = diff.new_value_id;
      diff.old_value_id = diff.new_value_id = value_id;
    }

    void erase(rope_state_diff & diff) {
      J_ASSERT(diff.old_value_id == value_id_before, "Value id before mismatch");
      diff.old_value_id = value_id;
      diff.metrics_d -= value.size();
    }

    rope_part split(rope_state_diff & diff, i32_t at) {
      J_ASSERT_RANGE(1, at, value.size());
      rope_part res(value.without_prefix(at));
      value = value.prefix(at);
      res.value_id = value_id;
      diff.old_value_id = value_id;
      value_id = ++g_value_id;
      diff.new_value_id = value_id;
      diff.metrics_d -= res.value.size();
      res.value_id_before = value_id;
      return res;
    }

    void erase_prefix(rope_state_diff & diff, i32_t len) {
      value_id_before = diff.new_value_id;
      diff.old_value_id = value_id;
      diff.new_value_id = value_id;
      J_ASSERT((i32_t)len < value.size(), "Out of range");
      value.remove_prefix(len);
      diff.metrics_d -= len;
    }

    void erase_suffix(rope_state_diff & diff, i32_t len) {
      J_ASSERT(diff.old_value_id == value_id_before, "Value id different");
      diff.old_value_id = value_id;
      diff.new_value_id = value_id;
      J_ASSERT(len < value.size(), "Out of range");
      value.remove_suffix(len);
      diff.metrics_d -= len;
    }

    j::pair<rope_part, rope_metrics> erase_middle(rope_state_diff & diff, u32_t at, u32_t len) {
      J_ASSERT(len && (i32_t)(at + len) < value.size(), "Out of range");
      diff.metrics_d -= len;
      rope_part res(value.without_prefix(at + len));
      value = value.prefix(at);
      res.value_id = value_id;
      value_id = ++g_value_id;
      res.value_id_before = value_id;
      return {
        res,
        rope_metrics{ (i32_t)res.value.size() }
      };
    }
  };

  inline rope_state_diff::rope_state_diff(const rope_part * p) noexcept
    : old_value_id(p ? p->value_id : 0),
      new_value_id(p ? p->value_id : 0)
  {
  }

  [[nodiscard]] rope_metrics rope_ctrl::compute_metrics(const rope_part & value) const noexcept {
    return { (i32_t)value.value.size() };
  }

  using tr = t::rope_tree<rope_description>;

  template<typename T>
  void assert_rope_tree_contents(const tr & t, T const * contents, u32_t sz) {
    J_ASSERT_ROPE_TREE(t);
    try {
      if (!sz) {
        REQUIRE_UNARY(t.begin() == t.end());
        REQUIRE_UNARY(t.empty());
        return;
      }
      REQUIRE_UNARY(t.begin() != t.end());
      u32_t i = 0;
      const rope_part * prev = nullptr;
      u32_t sum = 0U;
      for (const rope_part & c : t) {
        REQUIRE_UNARY(sum == (u32_t)c.ref.node()->cumulative_metrics(c.ref.index()).key());
        REQUIRE_LT(i, sz);
        REQUIRE_UNARY(prev == c.ref.previous());
        if (prev) {
          REQUIRE_UNARY(prev->ref.next() == &c);
        }
        REQUIRE_UNARY(c.value == contents[i]);
        ++i;
        sum += c.value.size();
        REQUIRE_UNARY(c.value_id_before == (prev ? prev->value_id : 0ULL));
        prev = &c;
      }
    } catch (...) {
      J_DUMP_ROPE_TREE(t);
    }
  }

  template<u32_t I>
  void assert_rope_tree_contents(const tr & t, const char * const (& contents)[I]) {
    assert_rope_tree_contents(t, contents, I);
  }

  template<u32_t I>
  void assert_rope_tree_contents(const tr & t, const s::const_string_view (& contents)[I]) {
    assert_rope_tree_contents(t, contents, I);
  }

  const char * const single_level_values[]{"Hello", ", ", "World!", "And goodbye."};
  const char * const multi_level_values[]{
    "On",
    "two",
    "occasions",
    ",",
    "I",
    "have",
    "been",
    "asked,",
    "\"Pray,",
    "Mr. Babbage,",
    "if",
    "you",
    "put",
    "into",
    "the",
    "machine",
    "wrong",
    "figures,",
    "will",
    "the",
    "right",
    "answers",
    "come out?\"",
    "...",
    "I",
    "am",
    "not",
    "able",
    "rightly",
    "to",
    "apprehend",
    "the",
    "kind",
    "of",
    "confusion",
    "of",
    "ideas",
    "that",
    "could",
    "provoke",
    "such",
    "a",
    "question",
    ".",
  };
}

TEST_CASE("Empty") {
  tr tree;
  REQUIRE_UNARY(tree.empty());
  REQUIRE_UNARY(tree.begin() == tree.end());
  J_ASSERT_ROPE_TREE(tree);
}

TEST_CASE("Single value") {
  tr tree;
  const auto it = tree.emplace(0, "Test");
  REQUIRE_UNARY(!tree.empty());
  REQUIRE_UNARY(tree.begin() != tree.end());
  REQUIRE_UNARY(tree.begin() == it);
}

TEST_CASE("Single-level") {
  tr tree;
  u32_t sz = 0U;
  try {
    for (u32_t i = 0; i < 4; ++i) {
      const auto it = tree.emplace(sz, single_level_values[i]);
      REQUIRE_UNARY(it == tree.find(sz).first);
      sz += ::j::strlen(single_level_values[i]);
      assert_rope_tree_contents(tree, single_level_values, i + 1U);
    }
  } catch (...) {
    J_DUMP_ROPE_TREE(tree);
    throw;
  }
}

TEST_CASE("Single-level find") {
  tr tree;
  for (u32_t i = 0U, sz = 0U; i < 4; ++i) {
    tree.emplace(sz, single_level_values[i]);
    sz += ::j::strlen(single_level_values[i]);
  }
  u32_t sz = 0U;
  for (auto v : single_level_values) {
    for (u32_t i = 0; i < ::j::strlen(v); ++i, ++sz) {
      const auto p = tree.find(sz);
      REQUIRE(p.first->value == v);
      REQUIRE(p.second == i);
    }
  }
}

TEST_CASE("Multi-level find") {
  tr tree;
  for (u32_t i = 0U, sz = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    const auto it = tree.emplace(sz, multi_level_values[i]);
    REQUIRE_UNARY(it == tree.find(sz).first);
    sz += ::j::strlen(multi_level_values[i]);
  }
  u32_t sz = 0U;
  for (auto v : multi_level_values) {
    for (u32_t i = 0; i < ::j::strlen(v); ++i, ++sz) {
      const auto p = tree.find(sz);
      REQUIRE(p.first->value == v);
      REQUIRE(p.second == i);
    }
  }
}

TEST_CASE("Single-level prepend") {
  tr tree;
  for (u32_t i = 0; i < 4; ++i) {
    const auto it = tree.emplace(0, single_level_values[3U - i]);
    REQUIRE_UNARY(it == tree.begin());
    assert_rope_tree_contents(tree, single_level_values + 3U - i, i + 1U);
  }
}

TEST_CASE("Single-level delete") {
  for (u32_t dir = 0; dir < 2; ++dir) {
    for (u32_t i = 1; i <= 4; ++i) {
      tr tree;
      j::vector<const char *> values;
      for (u32_t j = 0U; j < i; ++j) {
        tree.emplace(0, single_level_values[j]);
        values.push_front(single_level_values[j]);
      }
      assert_rope_tree_contents(tree, values.begin(), i);
      if (dir) {
        for (u32_t j = i - 1U; j < i; --j) {
          values.erase(j);
          u32_t sz = 0U;
          for (auto s : values) {
            sz += ::j::strlen(s);
          }
          tree.erase(tree.find(sz).first);
          assert_rope_tree_contents(tree, values.begin(), j);
        }
      } else {
        for (u32_t j = 0U; j < i; ++j) {
          tree.erase(tree.begin());
          values.erase(0U);
          assert_rope_tree_contents(tree, values.begin(), i - j - 1);
        }
      }
    }
  }
}

TEST_CASE("Multi-level") {
  tr tree;
  u32_t sz = 0U;
  for (u32_t i = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    const auto it = tree.emplace(sz, multi_level_values[i]);
    REQUIRE_UNARY(it == tree.find(sz).first);
    sz += ::j::strlen(multi_level_values[i]);
    assert_rope_tree_contents(tree, multi_level_values, i + 1U);
  }
}

TEST_CASE("Multi-level append") {
  tr tree;
  u32_t sz = 0U;
  for (u32_t i = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    const auto it = tree.emplace_append(multi_level_values[i]);
    REQUIRE_UNARY(it == tree.find(sz).first);
    sz += ::j::strlen(multi_level_values[i]);
    assert_rope_tree_contents(tree, multi_level_values, i + 1U);
  }
}

TEST_CASE("Multi-level prepend") {
  tr tree;
  for (u32_t i = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    const auto it = tree.emplace(0U, multi_level_values[J_ARRAY_SIZE(multi_level_values) - 1U - i]);
    REQUIRE_UNARY(it == tree.begin());
    assert_rope_tree_contents(tree, multi_level_values + J_ARRAY_SIZE(multi_level_values) - 1U - i, i + 1U);
  }
}

TEST_CASE("Multi-level prepend 2") {
  tr tree;
  for (u32_t i = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    const auto it = tree.emplace_prepend(multi_level_values[J_ARRAY_SIZE(multi_level_values) - 1U - i]);
    REQUIRE_UNARY(it == tree.begin());
    assert_rope_tree_contents(tree, multi_level_values + J_ARRAY_SIZE(multi_level_values) - 1U - i, i + 1U);
  }
}

TEST_CASE("Multi-level delete") {
  for (u32_t dir = 0U; dir < 2U; ++dir) {
    for (u32_t i = 1U; i <= J_ARRAY_SIZE(multi_level_values); ++i) {
      u32_t sz = 0U;
      tr tree;
      j::vector<const char *> values;
      for (u32_t j = 0U; j < i; ++j) {
        tree.emplace(sz, multi_level_values[j]);
        sz += ::j::strlen(multi_level_values[j]);
        values.push_back(multi_level_values[j]);
      }
      if (dir) {
        for (u32_t j = i - 1U; j < i; --j) {
          values.erase(j);
          u32_t sz = 0U;
          for (auto s : values) {
            sz += ::j::strlen(s);
          }
          tree.erase(tree.find(sz).first);
          assert_rope_tree_contents(tree, values.begin(), j);
        }
      } else {
        for (u32_t j = 0U; j < i; ++j) {
          tree.erase(tree.begin());
          values.erase(0U);
          assert_rope_tree_contents(tree, values.begin(), i - j - 1);
        }
      }

    }
  }
}

TEST_CASE("Multi-level delete 2") {
  for (u32_t i = 1U; i <= J_ARRAY_SIZE(multi_level_values); ++i) {
    for (u32_t k = 0U; k < i; ++k) {
      u32_t sz = 0U;
      tr tree;
      j::vector<const char *> values;
      for (u32_t j = 0U; j < i; ++j) {
        tree.emplace(sz, multi_level_values[j]);
        sz += ::j::strlen(multi_level_values[j]);
        values.push_back(multi_level_values[j]);
      }
      values.erase(k);
      u32_t sz2 = 0U;
      for (u32_t j = 0U; j < k; ++j) {
        sz2 += ::j::strlen(values[j]);
      }
      tree.erase(tree.find(sz2).first);
      assert_rope_tree_contents(tree, values.begin(), i - 1U);
    }
  }
}

TEST_CASE("Multi-level delete by iterator") {
  for (u32_t i = 1U; i <= J_ARRAY_SIZE(multi_level_values); ++i) {
    for (u32_t k = 0U; k < i; ++k) {
      u32_t sz = 0U;
      tr tree;
      j::vector<const char *> values;
      for (u32_t j = 0U; j < i; ++j) {
        tree.emplace(sz, multi_level_values[j]);
        sz += ::j::strlen(multi_level_values[j]);
        values.push_back(multi_level_values[j]);
      }
      try {
        values.erase(k);
        u32_t sz2 = 0U;
        for (u32_t j = 0U; j < k; ++j) {
          sz2 += ::j::strlen(values[j]);
        }
        auto it = tree.erase(tree.find(sz2).first);
        REQUIRE_UNARY(it == (values.empty() ? tree.end() : tree.find(sz2).first));
        assert_rope_tree_contents(tree, values.begin(), i - 1U);
      } catch (...) {
        J_DUMP_ROPE_TREE(tree);
        throw;
      }
    }
  }
}

TEST_CASE("Multi-level delete all by iterator") {
  tr tree;
  u32_t sz = 0U;
  for (u32_t i = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    tree.emplace(sz, multi_level_values[i]);
    sz += ::j::strlen(multi_level_values[i]);
  }
  auto it = tree.begin();
  for (u32_t i = 0U; i < J_ARRAY_SIZE(multi_level_values); ++i) {
    REQUIRE_UNARY(it != tree.end());
    it = tree.erase(it);
    assert_rope_tree_contents(tree, multi_level_values + i + 1U, J_ARRAY_SIZE(multi_level_values) - i - 1U);
  }
  REQUIRE_UNARY(it == tree.end());
}

TEST_CASE("Split single level") {
  tr tree;
  tree.emplace(0U, "Hello, World!");
  tree.split(7U);
  assert_rope_tree_contents(tree, {"Hello, ", "World!"});
  tree.split(10U);
  assert_rope_tree_contents(tree, {"Hello, ", "Wor", "ld!"});
  tree.split(1U);
  assert_rope_tree_contents(tree, {"H", "ello, ", "Wor", "ld!"});
}

TEST_CASE("Split multi-level") {
  tr tree;
  tree.emplace(0U, "Foo");
  tree.emplace(3U, "Bar");
  tree.emplace(6U, "Hello, ");
  tree.emplace(13U, "World!");
  auto it = tree.split(2U);
  REQUIRE_UNARY(it == tree.begin());
  assert_rope_tree_contents(tree, {"Fo", "o", "Bar", "Hello, ", "World!"});
  it = tree.split(9U);
  REQUIRE_UNARY(it == tree.find(6).first);
  assert_rope_tree_contents(tree, {"Fo", "o", "Bar", "Hel", "lo, ", "World!"});
  it = tree.split(7U);
  REQUIRE_UNARY(it == tree.find(6).first);
  assert_rope_tree_contents(tree, {"Fo", "o", "Bar", "H", "el", "lo, ", "World!"});
  it = tree.split(15U);
  REQUIRE_UNARY(it == tree.find(13).first);
  assert_rope_tree_contents(tree, {"Fo", "o", "Bar", "H", "el", "lo, ", "Wo", "rld!"});
  it = tree.split(16U);
  REQUIRE_UNARY(it == tree.find(15).first);
  assert_rope_tree_contents(tree, {"Fo", "o", "Bar", "H", "el", "lo, ", "Wo", "r", "ld!"});
  it = tree.split(18U);
  REQUIRE_UNARY(it == tree.find(16).first);
  assert_rope_tree_contents(tree, {"Fo", "o", "Bar", "H", "el", "lo, ", "Wo", "r", "ld", "!"});
}

TEST_CASE("Range erase") {
  for (u32_t start = 0U; start < 17U; ++start) {
    for (u32_t len = 1U; len <= 17U - start; ++len) {
      s::const_string_view values[]{
        "Hello,",
        "cruel",
        "World!",
      };
      tr tree;
      tree.emplace(0U, values[0]);
      tree.emplace(6U, values[1]);
      tree.emplace(11U, values[2]);
      tree.erase(start, len);
      j::vector<s::const_string_view> expected;
      i32_t skip = start, del = len;
      for (auto v : values) {
        if (skip >= v.size()) {
          expected.push_back(v);
          skip -= v.size();
          continue;
        }
        if (skip) {
          expected.push_back(v.prefix(skip));
        }
        v.remove_prefix(skip);
        skip = 0U;
        if (del >= v.size()) {
          del -= v.size();
          continue;
        }
        v.remove_prefix(del);
        del = 0U;
        expected.push_back(v);
      }
      assert_rope_tree_contents(tree, expected.begin(), expected.size());
    }
  }
}

TEST_CASE("Split erase") {
  const s::const_string_view values[]{
    "Hello,",
    "darkness",
    "my old",
    "friend",
  };
  u32_t pos = 0;
  for (u32_t i = 0; i < J_ARRAY_SIZE(values); ++i) {
    j::vector strings{values};
    strings.emplace(strings.begin() + i, strings[i].take_prefix(4).prefix(2));

    tr tree;
    for (auto v : values) {
      tree.emplace_append(v);
    }
    assert_rope_tree_contents(tree, values);
    REQUIRE_EQ(tree.root.depth(), 1U);

    tree.erase(pos + 2, 2);

    assert_rope_tree_contents(tree, strings.begin(), strings.size());
    REQUIRE_EQ(tree.root.depth(), 2U);

    pos += values[i].size();
  }
}

TEST_CASE("Split insert") {
  tr tree;
  tree.emplace(0U, "Hello World!");
  tree.emplace(5U, ", ");
  assert_rope_tree_contents(tree, {"Hello", ", ", " World!"});
}

TEST_SUITE_END();
