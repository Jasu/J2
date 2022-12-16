#pragma once

#include "containers/trees/rope_tree_node_fwd.hpp"

namespace j::inline containers::trees {
  /// Store nodes with computed intermediate keys in a B-tree-like structure.
  ///
  /// The structure extends the B-tree by:
  ///
  ///  * Storing sums of keys on the left, rather than keys faling between the
  ///    values on either sides.
  ///  * Storing the final sum, instead of just the dividing sums.
  ///  * Storing pointers to next / previous siblings in inner / leaf nodes.
  ///  * Storing a parent pointer and index in the parent in inner nodes.
  ///  * Storing a parent pointer and index in the parent in data elements.
  ///  * Storing additional metrics with keys (e.g. number of line breaks).
  ///
  ///  None of this changes the theoretical properties of a B-tree to the worse.
  ///  Updates of the additional elements is either O(h) or O(1), and storage
  ///  requirements for them are O(n).
  ///  The next/previous pointers make finding the next element O(1)
  ///  instead of O(h).
  ///
  /// Structure:
  ///
  ///              +-[Root]-+--------+--------------------+-----------------+
  ///              |   20   |   27   |          60        |         95      |
  ///              +--------+--------+--------------------+-----------------+
  ///                   |        |              |                   |
  ///     [Inner]       |        |              |                   |
  ///     +---+----+----+    +---+---+    +---+----+----+    +----+----+----+
  ///     | 7 | 11 | 20 |<-->| 2 | 7 |<-->| 8 | 26 | 33 |<-->| 17 | 27 | 35 |
  ///     +---+----+----+    +---+---+    +---+----+----+    +----+----+----+
  ///       |   |    |         |   |        |   |    |         |    |    |
  ///       |   |    |         |   |        |   |    |         |    |    |
  ///      .-. .-.  .-.       .-. .-.      .-. .--. .-.       .--. .--. .-.
  ///      |7| |4|  |9|       |2| |5|      |8| |18| |7|       |17| |10| |8|
  ///      '-' '-'  '-'       '-' '-'      '-' '--' '-'       '--' '--' '-'
  ///
  ///
  /// Additionally, value nodes are notified when the state of the previous node
  /// changes - i.e. if a node is deleted, inserted, or modified before the node.
  /// The node must then indicate its new metrics, and whether the state at the
  /// end of the node changed as well. If the state at the end changed, the change
  /// is propagated forward to the next sibling.
  template<RopeTreeDescription DescriptionT>
  class rope_tree final {
  public:
    using node_t         J_NO_DEBUG_TYPE = rope_tree_node<DescriptionT>;
    using key_t          J_NO_DEBUG_TYPE = typename DescriptionT::key_t;
    using key_diff_t     J_NO_DEBUG_TYPE = typename DescriptionT::key_diff_t;
    using metrics_t      J_NO_DEBUG_TYPE = typename DescriptionT::metrics_t;
    using value_t        J_NO_DEBUG_TYPE = typename DescriptionT::value_t;
    using state_diff_t   J_NO_DEBUG_TYPE = typename DescriptionT::state_diff_t;
    using iterator       J_NO_DEBUG_TYPE = rope_tree_iterator<node_t, false>;
    using const_iterator J_NO_DEBUG_TYPE = rope_tree_iterator<node_t, true>;

    J_A(AI,NODISC,NE) bool empty() const noexcept                { return root.empty(); }

    iterator begin() noexcept                  { return root.begin(); }
    const_iterator begin() const noexcept      { return const_cast<rope_tree*>(this)->begin(); }

    iterator end() noexcept                    { return root.end(); }
    const_iterator end() const noexcept        { return const_cast<rope_tree*>(this)->end(); }

    iterator before_end() noexcept             { return root.before_end(); }
    const_iterator before_end() const noexcept { return const_cast<rope_tree*>(this)->before_end(); }

    J_INLINE_GETTER const metrics_t & metrics() const noexcept
    { return root.metrics(); }

    pair<iterator, key_t> find(key_t key);

    J_INLINE_GETTER pair<const_iterator, key_t> find(key_t key) const {
      auto p = const_cast<rope_tree*>(this)->find(key);
      return { p.first, p.second };
    }

    template<typename Key>
    J_INLINE_GETTER pair<iterator, key_t> find_by_secondary(Key key)
    { return root.find_by_secondary(key); }

    template<typename Key>
    J_INLINE_GETTER pair<const_iterator, key_t> find_by_secondary(Key key) const {
      auto p = const_cast<rope_tree*>(this)->find_by_secondary<Key>(key);
      return { p.first, p.second };
    }

    iterator split(key_t key);

    iterator split(const_iterator it, key_t offset);

    template<typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG iterator emplace_at(const_iterator it, key_t key, Args && ... args) {
      const iterator & it_ = reinterpret_cast<const iterator &>(it);
      return it_.node->emplace_at(++modification_stamp, it_, key, static_cast<Args &&>(args)...);
    }

    /// Emplace a node at key. May split existing nodes.
    template<typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG iterator emplace(key_t key, Args && ... args) {
      auto p = find(key);
      return emplace_at(p.first, p.second, static_cast<Args &&>(args)...);
    }

    template<typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG iterator emplace(const_iterator it, Args && ... args) {
      const iterator & it_ = reinterpret_cast<const iterator &>(it);
      return it_.node->emplace_before(++modification_stamp, it_, static_cast<Args &&>(args)...);
    }

    /// Emplace a node at the end.
    template<typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG iterator emplace_prepend(Args && ... args) {
      return emplace(begin(), static_cast<Args &&>(args)...);
    }

    /// Emplace a node at the end.
    template<typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG iterator emplace_append(Args && ... args) {
      return emplace(end(), static_cast<Args &&>(args)...);
    }

    iterator erase(const_iterator it, key_t offset, key_t sz);

    iterator erase(key_t start, key_t sz);

    /// Erase a node from the tree.
    iterator erase(const_iterator it);

    u32_t modification_stamp_between(const_iterator first, const_iterator last) noexcept;

    J_INLINE_GETTER u32_t new_modification_stamp() noexcept
    { return ++modification_stamp; }

    node_t root;
    u32_t modification_stamp = 0U;
  };
}

#define J_DECLARE_EXTERN_ROPE_TREE(DESC)                            \
  extern template struct j::containers::trees::rope_tree_iterator<  \
    j::containers::trees::rope_tree_node<DESC>, true>;              \
  extern template struct j::containers::trees::rope_tree_iterator<  \
    j::containers::trees::rope_tree_node<DESC>, false>;             \
  extern template class j::containers::trees::rope_tree_node<DESC>; \
  extern template class j::containers::trees::rope_tree<DESC>
