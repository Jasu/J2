#pragma once

#include "containers/pair.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::inline containers::trees {
  /// Base class for binary tree nodes.
  ///
  /// Holds pointers to left, right, and parent and space for book-keeping information.
  /// Does not contain key or value - at least key should be added by a child class.
  struct binary_tree_node_base {
    binary_tree_node_base * left = nullptr;
    binary_tree_node_base * right = nullptr;
    uptr_t m_parent_uptr:48 = 0;
    uptr_t node_data:15 = 0;
    uptr_t m_is_right:1 = 0;

    J_ALWAYS_INLINE constexpr binary_tree_node_base() noexcept = default;

    J_INLINE_GETTER bool is_left_child() const noexcept
    { return m_parent_uptr && !m_is_right; }

    J_INLINE_GETTER bool is_right_child() const noexcept
    { return m_is_right; }

    J_INLINE_GETTER binary_tree_node_base * parent() const noexcept
    { return reinterpret_cast<binary_tree_node_base *>(m_parent_uptr); }

    void set_left(binary_tree_node_base * l) noexcept {
      J_ASSUME(l != this);
      if ((left = l)) {
        l->m_parent_uptr = reinterpret_cast<uptr_t>(this);
        l->m_is_right = false;
      }
    }

    void set_right(binary_tree_node_base * r) noexcept {
      J_ASSUME(r != this);
      if ((right = r)) {
        r->m_parent_uptr = reinterpret_cast<uptr_t>(this);
        r->m_is_right = true;
      }
    }

    /// Calculate the depth of the branch. For debug purposes.
    u32_t depth() const noexcept;

    void destroy() noexcept;

    binary_tree_node_base(binary_tree_node_base &&) = delete;
    binary_tree_node_base & operator=(binary_tree_node_base &&) = delete;
    binary_tree_node_base(const binary_tree_node_base &) = delete;
    binary_tree_node_base & operator=(const binary_tree_node_base &) = delete;
  };

  /// Base class for threaded binary tree nodes, i.e. nodes that contain next/previous pointers.
  ///
  /// \note "Threaded" means additional iteration pointers, and is not related to multi-threading.
  struct threaded_binary_tree_node_base : public binary_tree_node_base {
    using threaded_binary_tree_node_tag J_NO_DEBUG_TYPE = void;
    threaded_binary_tree_node_base * previous = nullptr;
    threaded_binary_tree_node_base * next = nullptr;
  };

  template<typename N, typename = void>
  struct J_TYPE_HIDDEN is_threaded_node final {
    J_NO_DEBUG static constexpr inline bool value = false;
  };

  template<typename N>
  struct J_TYPE_HIDDEN is_threaded_node<N, typename N::threaded_binary_tree_node_tag> final {
    J_NO_DEBUG static constexpr inline bool value = true;
  };

  template<typename N>
  J_NO_DEBUG inline constexpr bool is_threaded_node_v = is_threaded_node<N>::value;

  /// Default implementation for a binary tree node.
  template<typename KeyT, typename ValueT>
  struct binary_tree_node final : public binary_tree_node_base {
    template<typename K, typename... Vs>
    J_ALWAYS_INLINE explicit binary_tree_node(K && key, Vs && ... values) noexcept
      : key(static_cast<K &&>(key)),
        value(static_cast<Vs &&>(values)...)
    { }

    KeyT key;
    ValueT value;
  };

  /// Find the successor node of n.
  template<typename N>
  N * successor(N * J_NOT_NULL const n) noexcept {
    if constexpr (is_threaded_node_v<N>) {
      return static_cast<N*>(n->next);
    } else {
      if (auto res = n->right) {
        while (res->left) { res = res->left; }
        return static_cast<N*>(res);
      } else if (n->is_left_child()) {
        return static_cast<N*>(n->parent());
      } else {
        const binary_tree_node_base * res = n;
        while (res->is_right_child()) { res = res->parent(); }
        return const_cast<N*>(static_cast<const N*>(res->parent()));
      }
    }
  }

  /// Find the successor node of n.
  template<typename N>
  N * predecessor(N * J_NOT_NULL const n) noexcept {
    if constexpr (is_threaded_node_v<N>) {
      return static_cast<N*>(n->previous);
    } else {
      if (auto res = n->left) {
        while (res->right) { res = res->right; }
        return static_cast<N*>(res);
      } else if (n->is_right_child()) {
        return static_cast<N*>(n->parent());
      } else {
        const binary_tree_node_base * res = n;
        while (res->is_left_child()) { res = res->parent(); }
        return const_cast<N*>(static_cast<const N*>(res->parent()));
      }
    }
  }


  /// Swap binary tree node positions - does not swap keys.
  template<typename N>
  void swap_positions(N * J_NOT_NULL const a, N * J_NOT_NULL const b) noexcept {
    J_ASSERT(a != b, "Cannot swap with self.");
    {
      const u32_t old_data = a->node_data, old_is_right = a->m_is_right;
      a->node_data = b->node_data, a->m_is_right = b->m_is_right;
      b->node_data = old_data, b->m_is_right = old_is_right;
    }

    if constexpr (is_threaded_node_v<N>) {
      threaded_binary_tree_node_base * const old_next = a->next,
                                     * const old_previous = a->previous;
      a->next = (b->next == a) ? b : b->next;
      a->previous = (b->previous == a) ? b : b->previous;
      b->next = (old_next == b) ? a : old_next;
      b->previous = (old_previous == b) ? a : old_previous;

      if (a->next)     { a->next->previous = a; }
      if (a->previous) { a->previous->next = a; }
      if (b->next)     { b->next->previous = b; }
      if (b->previous) { b->previous->next = b; }
    }

    binary_tree_node_base * const old_left = a->left, * const old_right = a->right;
    const uptr_t old_parent = a->m_parent_uptr;

    a->m_parent_uptr = b->parent() == a ? reinterpret_cast<uptr_t>(b) : b->m_parent_uptr;
    a->left = (b->left == a) ? b : b->left;
    a->right = (b->right == a) ? b : b->right;

    b->m_parent_uptr = (old_parent == reinterpret_cast<uptr_t>(b))
      ? reinterpret_cast<uptr_t>(a) : old_parent;
    b->left = (old_left == b) ? a : old_left;
    b->right = (old_right == b) ? a : old_right;

    if (a->left)  { a->left->m_parent_uptr = reinterpret_cast<uptr_t>(a); }
    if (a->right) { a->right->m_parent_uptr = reinterpret_cast<uptr_t>(a); }
    if (b->left)  { b->left->m_parent_uptr = reinterpret_cast<uptr_t>(b); }
    if (b->right) { b->right->m_parent_uptr = reinterpret_cast<uptr_t>(b); }

    if (a->parent()) {
      (a->m_is_right ? a->parent()->right : a->parent()->left) = a;
    }
    if (b->parent()) {
      (b->m_is_right ? b->parent()->right : b->parent()->left) = b;
    }
  }

  template<typename Cmp, typename N, typename K>
  N * find(const K & k, N * n) noexcept {
    constexpr Cmp cmp;
    while (n) {
      auto c = cmp(n->key, k);
      if (c == 0) {
        return n;
      }
      n = static_cast<N*>(c < 0 ? n->right : n->left);
    }
    return nullptr;
  }

  template<typename Cmp, typename N, typename K>
  N * lower_bound(const K & k, N * n) noexcept {
    constexpr Cmp cmp;
    N * bound = nullptr;
    while (n) {
      auto c = cmp(n->key, k);
      if (c <= 0) {
        bound = n;
        if (c == 0) {
          break;
        }
      }
      n = static_cast<N*>(c < 0 ? n->right : n->left);
    }
    return bound;
  }

  template<typename Cmp, typename N, typename K, typename... Vs>
  pair<N*, bool> insert(N * J_NOT_NULL n, const K & k, Vs && ... vs) noexcept {
    constexpr Cmp cmp;
    for (;;) {
      const auto c = cmp(n->key, k);
      if (J_UNLIKELY(c == 0)) {
        return {n, false};
      } else if (const auto next = static_cast<N*>(c < 0 ? n->right : n->left)) {
        n = next;
      } else {
        const auto ins = ::new N(k, static_cast<Vs &&>(vs)...);
        if (c < 0) {
          n->set_right(ins);
          if constexpr (is_threaded_node_v<N>) {
            ins->previous = n;
            if ((ins->next = n->next)) {
              ins->next->previous = ins;
            }
            n->next = ins;
          }
        } else {
          n->set_left(ins);
          if constexpr (is_threaded_node_v<N>) {
            ins->next = n;
            if ((ins->previous = n->previous)) {
              ins->previous->next = ins;
            }
            n->previous = ins;
          }
        }
        return {ins, true};
      }
    }
  }

  enum class binary_tree_iterator_side : i8_t {
    in_range = 0,
    before_begin = -1,
    at_end = 1,
  };

  template<typename N, typename Cmp>
  struct binary_tree_iterator {
    N * node = nullptr;
    binary_tree_iterator_side side = binary_tree_iterator_side::at_end;

    J_INLINE_GETTER_NONNULL N * operator->() const noexcept {
      J_ASSERT(node && side == binary_tree_iterator_side::in_range,
               "Dereferenced binary_tree_iterator out of bounds.");
      return node;
    }
    J_INLINE_GETTER N & operator*() const noexcept { return *operator->(); }

    binary_tree_iterator & operator++() noexcept;
    binary_tree_iterator operator++(int) noexcept;

    binary_tree_iterator & operator--() noexcept;
    binary_tree_iterator operator--(int) noexcept;

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return side == binary_tree_iterator_side::in_range; }
    J_INLINE_GETTER bool operator!() const noexcept
    { return side != binary_tree_iterator_side::in_range; }

    J_INLINE_GETTER operator binary_tree_iterator<const N, Cmp>() const noexcept {
      return binary_tree_iterator<const N, Cmp>{node, side};
    }

    template<typename N2>
    J_INLINE_GETTER bool operator==(const binary_tree_iterator<N2, Cmp> & rhs) const noexcept
    { return node == rhs.node && side == rhs.side; }

    template<typename N2>
    J_INLINE_GETTER bool operator<(const binary_tree_iterator<N2, Cmp> & rhs) const noexcept {
      constexpr Cmp cmp;
      return side < rhs.side ||
        (side == rhs.side && node && rhs.node && cmp(node->key, rhs.node->key) < 0);
    }

    template<typename N2>
    J_INLINE_GETTER bool operator>=(const binary_tree_iterator<N2, Cmp> & rhs) const noexcept {
      return !operator<(rhs);
    }

    template<typename N2>
    J_INLINE_GETTER bool operator>(const binary_tree_iterator<N2, Cmp> & rhs) const noexcept {
      return rhs.operator<(*this);
    }

    template<typename N2>
    J_INLINE_GETTER bool operator<=(const binary_tree_iterator<N2, Cmp> & rhs) const noexcept {
      return !rhs.operator<(*this);
    }

    J_ALWAYS_INLINE void reset() noexcept {
      side = binary_tree_iterator_side::at_end;
      node = nullptr;
    }

    J_INLINE_GETTER bool is_end() const noexcept {
      return side == binary_tree_iterator_side::at_end;
    }

    J_INLINE_GETTER bool is_before_begin() const noexcept {
      return side == binary_tree_iterator_side::before_begin;
    }

    J_INLINE_GETTER binary_tree_iterator as_end() const noexcept {
      return { node, binary_tree_iterator_side::at_end };
    }
  };

  template<typename N, typename Cmp>
  binary_tree_iterator<N, Cmp> & binary_tree_iterator<N, Cmp>::operator++() noexcept {
    J_ASSERT(node && side != binary_tree_iterator_side::at_end,
             "Tried to increment out of bounds.");
    if (J_UNLIKELY(side == binary_tree_iterator_side::before_begin)) {
      side = binary_tree_iterator_side::in_range;
    } else if (const auto succ = successor(node)) {
      node = succ;
    } else {
      side = binary_tree_iterator_side::at_end;
    }
    return *this;
  }

  template<typename N, typename Cmp>
  binary_tree_iterator<N, Cmp> & binary_tree_iterator<N, Cmp>::operator--() noexcept {
    J_ASSERT(node && side != binary_tree_iterator_side::before_begin,
             "Decremented out of bounds.");
    if (J_UNLIKELY(side == binary_tree_iterator_side::at_end)) {
      side = binary_tree_iterator_side::in_range;
    } else if (const auto pred = predecessor(node)) {
      node = pred;
    } else {
      side = binary_tree_iterator_side::before_begin;
    }
    return *this;
  }

  template<typename N, typename Cmp>
  binary_tree_iterator<N, Cmp> binary_tree_iterator<N, Cmp>::operator++(int) noexcept {
    const binary_tree_iterator result{*this};
    return operator++(), result;
  }

  template<typename N, typename Cmp>
  binary_tree_iterator<N, Cmp> binary_tree_iterator<N, Cmp>::operator--(int) noexcept {
    const binary_tree_iterator result{*this};
    return operator--(), result;
  }

}
