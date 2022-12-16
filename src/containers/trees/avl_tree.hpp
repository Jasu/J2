#pragma once

#include "containers/trees/avl_tree_fwd.hpp"

namespace j::inline containers::trees {

  /// Rotate the right child of node n to the place of n.
  ///
  ///
  ///          (N)                  (R)
  ///          / \                  / \.
  ///         /   \                /   \.
  ///        /     \              /     \.
  ///      (l)     (R)  ==TO=>  (N)     (rr)
  ///              / \          / \.
  ///             /   \        /   \.
  ///           (rl) (rr)    (l)   (rl)
  ///
  /// - Before: BalanceFactor(N) = Depth(l) - (1 + Max(Depth(rl), Depth(rr)))
  ///                            = Depth(l) - 1 - Max(Depth(rl), Depth(rr))
  /// - After: BalanceFactor(R) = 1 + Max(Depth(l), Depth(rl)) - Depth(rr)
  ///
  /// -> BalanceFactor(N) gets incremented (shifted towards left)
  /// -> BalanceFactor(R) MIGHT get incremented (shifted towards left):
  ///    - If Depth(rr) > Depth(rl), then BalanceFactor(R) does not change
  ///    - If Depth(rr) <= Depth(rl), then BalanceFactor(R) increments.
  template<typename T>
  J_RETURNS_NONNULL T * rotate_l_impl(T * n, T ** root) noexcept {
    J_ASSUME_NOT_NULL(n, root);
    auto * new_root = n->right;
    J_ASSUME_NOT_NULL(new_root);
    J_ASSERT(new_root->parent() == n && new_root->is_right_child(), "Parent mismatch.");
    n->set_right(new_root->left);
    if (n->is_left_child()) {
      J_ASSERT(n->parent()->left == n, "Parent mismatch.");
      n->parent()->set_left(new_root);
    } else if (n->is_right_child()) {
      J_ASSERT(n->parent()->right == n, "Parent mismatch.");
      n->parent()->set_right(new_root);
    } else {
      *root = static_cast<T*>(new_root);
      new_root->m_parent_uptr = 0;
      new_root->m_is_right = false;
    }
    new_root->set_left(n);
    return static_cast<T*>(new_root);
  }

  template<typename T>
  J_RETURNS_NONNULL T * rotate_l(T * n, T ** root) noexcept {
    auto new_root = rotate_l_impl(n, root);
    if (new_root->node_data == 1) {
      n->node_data = 2;
    } else {
      J_ASSUME(new_root->node_data == 2);
      n->node_data = 1;
    }
    --(new_root->node_data);
    return new_root;
  }

  /// Rotate the left child of node n to the place of n.
  ///
  ///
  ///          (N)                   (L)
  ///          / \                   / \.
  ///         /   \                 /   \.
  ///        /     \               /     \.
  ///      (L)     (r)  ==TO=>  (ll)     (N)
  ///      / \                           / \.
  ///     /   \                         /   \.
  ///   (ll) (lr)                    (lr)   (r)
  ///
  /// - Before: BalanceFactor(N) = 1 + Max(Depth(ll), Depth(lr)) - Depth(r)
  /// - After: BalanceFactor(L) = Depth(ll) - (1 + Max(Depth(lr), Depth(r)))
  ///
  /// -> BalanceFactor(N) gets decremented (shifted towards right)
  /// -> BalanceFactor(L) MIGHT get decremented (shifted towards right):
  ///    - If Depth(ll) > Depth(lr), then BalanceFactor(L) does not change
  ///    - If Depth(ll) <= Depth(lr), then BalanceFactor(L) decrements.
  template<typename T>
  J_RETURNS_NONNULL T * rotate_r_impl(T * n, T ** root) noexcept {
    J_ASSUME_NOT_NULL(n, root);
    auto * new_root = n->left;
    J_ASSUME_NOT_NULL(new_root);
    J_ASSERT(new_root->parent() == n && new_root->is_left_child(), "Parent mismatch.");
    n->set_left(new_root->right);
    if (n->is_left_child()) {
      J_ASSERT(n->parent()->left == n, "Parent mismatch.");
      n->parent()->set_left(new_root);
    } else if (n->is_right_child()) {
      J_ASSERT(n->parent()->right == n, "Parent mismatch.");
      n->parent()->set_right(new_root);
    } else {
      *root = static_cast<T*>(new_root);
      new_root->m_parent_uptr = 0;
      new_root->m_is_right = false;
    }
    new_root->set_right(n);
    return static_cast<T*>(new_root);
  }

  template<typename T>
  J_RETURNS_NONNULL T * rotate_r(T * n, T ** root) noexcept {
    auto new_root = rotate_r_impl(n, root);
    if (new_root->node_data == 1) {
      // New root (L) was balanced, but a node was added to right.
      n->node_data = 0;
    } else {
      J_ASSUME(new_root->node_data == 0);
      // New root (L) was not balanced, and a node was added to right.
      n->node_data = 1;
    }
    ++(new_root->node_data);
    return new_root;
  }

  /// Rotate the left-right grandchild of n to the place of n:
  ///
  /// 1. Left-rotate Left(n):
  ///
  ///           (N)                      (N)
  ///           / \                      / \.
  ///          /   \                    /   \.
  ///         /     \                  /     \.
  ///       (L)     (r)              (LR)    (r)
  ///       / \                      / \.
  ///      /   \                    /   \.
  ///     /     \                  /     \.
  ///   (ll)    (LR)     ==TO=>  (L)    (lrr)
  ///           / \              / \.
  ///          /   \            /   \.
  ///        (lrl) (lrr)      (ll) (lrl)
  ///
  /// 2. Right-rotate n:
  ///
  ///              (N)                  (LR)
  ///              / \.                 /  \.
  ///             /   \.               /    \.
  ///            /     \.             /      \.
  ///           /       \.           /        \.
  ///         (LR)      (r)        (L)         (N)
  ///         / \.                 / \         / \.
  ///        /   \.               /   \       /   \.
  ///       /     \.     ==TO=>  /     \     /     \.
  ///     (L)    (lrr)         (ll)   (lrl) (lrr)  (r)
  ///     / \.
  ///    /   \.
  ///  (ll) (lrl)
  ///
  template<typename T>
  J_RETURNS_NONNULL T * rotate_lr(T * n, T ** root) noexcept {
    J_ASSUME_NOT_NULL(n, root);
    auto l = static_cast<T*>(n->left);
    J_ASSUME_NOT_NULL(l);
    rotate_l_impl(l, root);
    auto new_root = rotate_r_impl(n, root);
    if (new_root->node_data == 0) {
      n->node_data = 2;
      l->node_data = 1;
    } else if (new_root->node_data == 1) {
      n->node_data = 1;
      l->node_data = 1;
    } else {
      n->node_data = 1;
      l->node_data = 0;
    }
    new_root->node_data = 1;
    return new_root;
  }

  /// Rotate the right-left grandchild of n to the place of n:
  ///
  /// 1.Right-rotate Right(n):
  ///
  ///           (N)                    (N)
  ///           / \                    / \.
  ///          /   \                  /   \.
  ///         /     \                /     \.
  ///       (l)     (R)            (l)     (RL)
  ///               / \                    / \.
  ///              /   \                  /   \.
  ///             /     \                /     \.
  ///           (RL)    (rr) ==TO=>   (rll)    (R)
  ///           / \                            / \.
  ///          /   \                          /   \.
  ///      (rll)  (rlr)                    (rlr)  (rr)
  ///
  /// 2. Left-rotate n:
  ///
  ///      (N)                        (RL)
  ///      / \.                       /  \.
  ///     /   \.                     /    \.
  ///    /     \.                   /      \.
  ///  (l)     (RL)              (N)       (R)
  ///          / \.              / \       / \.
  ///         /   \.  ==TO=>    /   \     /   \.
  ///        /     \.          /     \   /     \.
  ///     (rll)    (R)       (l)  (rll) (rlr)  (rr)
  ///              / \.
  ///             /   \.
  ///          (rlr)  (rr)
  template<typename T>
  J_RETURNS_NONNULL T * rotate_rl(T * n, T ** root) noexcept {
    J_ASSUME_NOT_NULL(n, root);
    auto r = static_cast<T*>(n->right);
    J_ASSUME_NOT_NULL(r);
    rotate_r_impl(r, root);
    auto new_root = rotate_l_impl(n, root);
    if (new_root->node_data == 2) {
      n->node_data = 0;
      r->node_data = 1;
    } else if (new_root->node_data == 1) {
      n->node_data = 1;
      r->node_data = 1;
    } else {
      n->node_data = 1;
      r->node_data = 2;
    }
    new_root->node_data = 1;
    return new_root;
  }

  template<typename K, typename V, typename Cmp, typename N>
  template<typename T, typename>
  pair<typename avl_tree<K, V, Cmp, N>::iterator, bool> avl_tree<K, V, Cmp, N>::insert(
    const K & key, const T & value)
  {
    return emplace(key, value);
  }

  template<typename K, typename V, typename Cmp, typename N>
  template<typename... Vs>
  pair<typename avl_tree<K, V, Cmp, N>::iterator, bool> avl_tree<K, V, Cmp, N>::emplace(const K & key, Vs && ... value) {
    if (!m_root) {
      m_root = ::new node_t(key, static_cast<Vs &&>(value)...);
      m_root->node_data = 1;
      return {iterator{m_root, binary_tree_iterator_side::in_range}, true};
    }
    const auto p = trees::insert<Cmp>(m_root, key, static_cast<Vs &&>(value)...);
    const auto n = p.first;
    if (J_UNLIKELY(!p.second)) {
      // No insert, already existed.
      return {iterator{n, binary_tree_iterator_side::in_range}, false};
    }
    n->node_data = 1;

    for (auto cur = n, parent = static_cast<node_t*>(n->parent()); parent; cur = parent, parent = static_cast<node_t*>(parent->parent())) {
      int parent_balance = (int)parent->node_data - 1;
      int cur_balance = (int)cur->node_data - 1;
      J_ASSERT(parent_balance >= -1 && parent_balance <= 1, "Balance out of range.");
      J_ASSERT(cur_balance >= -1 && cur_balance <= 1, "Balance out of range.");
      if (cur->is_left_child()) {
        if (parent_balance == -1) {
          if (cur_balance == 1) {
            rotate_lr(parent, &m_root);
          } else {
            rotate_r(parent, &m_root);
          }
          parent_balance = parent->node_data - 1;
        } else {
          --parent_balance;
        }
      } else {
        if (parent_balance == 1) {
          if (cur_balance == -1) {
            rotate_rl(parent, &m_root);
          } else {
            rotate_l(parent, &m_root);
          }
          parent_balance = parent->node_data - 1;
        } else {
          ++parent_balance;
        }
      }
      parent->node_data = parent_balance + 1;
      if (parent_balance == 0) {
        break;
      }
    }
    return {iterator{n, binary_tree_iterator_side::in_range}, true};
  }

  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::erase(const_iterator it) noexcept {
    auto n = const_cast<node_t*>(it.node);
    J_ASSUME_NOT_NULL(n);

    node_t * const succ = successor(n);
    if (n->left && n->right) {
      // Deleting a node with two children. Find the successor of the node to replace it with.
      J_ASSUME_NOT_NULL(succ);
      swap_positions(n, succ);
      if (!succ->parent()) { m_root = succ; }
    }

    node_t * p = static_cast<node_t*>(n->parent());
    node_t * const child = static_cast<node_t *>(n->left ? n->left : n->right);
    if constexpr (is_threaded_node_v<N>) {
      if (n->next) {
        n->next->previous = n->previous;
      }
      if (n->previous) {
        n->previous->next = n->next;
      }
    }
    bool is_left = n->is_left_child();
    ::delete n;
    if (!p) {
      // Deleted root, replaced with its only child. Since the child must already satisfy
      // the AVL property, no rotations are needed.
      if ((m_root = child)) {
        child->m_parent_uptr = 0;
        child->m_is_right = 0;
      }
    } else {
      is_left ? p->set_left(child) : p->set_right(child);
      for (; p; p = static_cast<node_t*>(p->parent())) {
        int parent_balance = (int)p->node_data - 1;
        J_ASSERT(parent_balance >= -1 && parent_balance <= 1, "Balance out of range");
        if (is_left) {
          if (parent_balance > 0) {
            // Deleted left node of a right-heavy subtree
            int right_balance = (int)p->right->node_data - 1;
            if (right_balance < 0) {
              p = rotate_rl(p, &m_root);
            } else {
              p = rotate_l(p, &m_root);
              if (!right_balance) {
                break;
              }
            }
          } else if (p->node_data++ == 1) {
            break;
          }
        } else {
          if (parent_balance < 0) {
            // Deleted right node of a left-heavy subtree
            int left_balance = (int)p->left->node_data - 1;
            if (left_balance > 0) {
              p = rotate_lr(p, &m_root);
            } else {
              p = rotate_r(p, &m_root);
              if (!left_balance) {
                break;
              }
            }
          } else if (p->node_data-- == 1) {
            break;
          }
        }
        is_left = p->is_left_child();
      }
    }
    return succ ? iterator{succ, binary_tree_iterator_side::in_range} : end();
  }

  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::find(const K & key) noexcept {
    const auto n = trees::find<Cmp>(key, m_root);
    return n ? iterator{n, binary_tree_iterator_side::in_range} : end();
  }
  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::lower_bound(const K & key) noexcept {
    const auto n = trees::lower_bound<Cmp>(key, m_root);
    return n ? iterator{n, binary_tree_iterator_side::in_range} : end();
  }

  template<typename K, typename V, typename Cmp, typename N>
  N & avl_tree<K, V, Cmp, N>::front() noexcept {
    J_ASSUME_NOT_NULL(m_root);
    auto n = m_root;
    while (n->left) { n = static_cast<node_t*>(n->left); }
    return *n;
  }

  template<typename K, typename V, typename Cmp, typename N>
  N & avl_tree<K, V, Cmp, N>::back() noexcept {
    J_ASSUME_NOT_NULL(m_root);
    auto n = m_root;
    while (n->right) { n = static_cast<node_t*>(n->right); }
    return *n;
  }

  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::before_begin() noexcept {
    if (!m_root) { return iterator{}; }
    return iterator{&front(), binary_tree_iterator_side::before_begin};
  }

  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::begin() noexcept {
    if (!m_root) { return iterator{}; }
    return iterator{&front(), binary_tree_iterator_side::in_range};
  }

  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::end() noexcept {
    if (!m_root) { return iterator{}; }
    return iterator{&back(), binary_tree_iterator_side::at_end};
  }

  template<typename K, typename V, typename Cmp, typename N>
  typename avl_tree<K, V, Cmp, N>::iterator avl_tree<K, V, Cmp, N>::before_end() noexcept {
    if (!m_root) { return iterator{}; }
    return iterator{&back(), binary_tree_iterator_side::in_range};
  }

  template<typename K, typename V, typename Cmp, typename N>
  void avl_tree<K, V, Cmp, N>::clear() noexcept {
    if constexpr (is_threaded_node_v<N>) {
      const auto e = end();
      for (auto it = begin(); it != e;) {
        auto * n = &*it;
        ++it;
        ::delete n;
      }
    } else {
      if (m_root) {
        m_root->destroy();
      }
    }
    m_root = nullptr;
  }
}

#define J_DEFINE_EXTERN_AVL_TREE(K, V)                                  \
  template struct j::containers::trees::binary_tree_node<K, V>;         \
  template struct j::containers::trees::binary_tree_iterator<           \
                    j::containers::trees::binary_tree_node<K, V>,       \
                    j::containers::comparer<K>>;                        \
  template struct j::containers::trees::binary_tree_iterator<           \
                    const j::containers::trees::binary_tree_node<K, V>, \
                    j::containers::comparer<K>>;                        \
  template class j::containers::trees::avl_tree<K, V>

#define J_DEFINE_EXTERN_AVL_TREE_NODE(K, V, N)                          \
  template struct j::containers::trees::binary_tree_iterator<           \
                    N, j::containers::comparer<K>>;                     \
  template struct j::containers::trees::binary_tree_iterator<           \
                    const N, j::containers::comparer<K>>;               \
  template class j::containers::trees::avl_tree<K, V, j::containers::comparer<K>, N>
