#pragma once

#include "containers/trees/binary_tree_node.hpp"
#include "containers/compare_fn.hpp"

namespace j::inline containers::trees {
  template<typename K,
           typename V,
           typename Cmp = comparer<K>,
           typename Node = binary_tree_node<K, V>>
  class avl_tree final {
  public:
    using node_t = Node;
    using iterator = binary_tree_iterator<node_t, Cmp>;
    using const_iterator = binary_tree_iterator<const node_t, Cmp>;

    J_ALWAYS_INLINE constexpr avl_tree() noexcept = default;
    J_ALWAYS_INLINE constexpr avl_tree(avl_tree && rhs) noexcept
      : m_root(rhs.m_root)
    { rhs.m_root = nullptr; }

    J_ALWAYS_INLINE avl_tree & operator=(avl_tree && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        clear();
        m_root = rhs.m_root;
        rhs.m_root = nullptr;
      }
      return *this;
    }

    template<typename T = V, typename = ::j::enable_if_t<::j::is_copy_constructible_v<T>>>
    pair<iterator, bool> insert(const K & key, const T & value);
    template<typename... Vs>
    pair<iterator, bool> emplace(const K & key, Vs && ... value);
    [[nodiscard]] iterator find(const K & key) noexcept;
    J_INLINE_GETTER const_iterator find(const K & key) const noexcept {
      return const_cast<avl_tree*>(this)->find(key);
    }

    [[nodiscard]] iterator lower_bound(const K & key) noexcept;
    J_INLINE_GETTER const_iterator lower_bound(const K & key) const noexcept {
      return const_cast<avl_tree*>(this)->lower_bound(key);
    }

    iterator erase(const_iterator It) noexcept;
    J_ALWAYS_INLINE void erase(const K & key) noexcept {
      erase(find(key));
    }

    [[nodiscard]] node_t & front() noexcept;
    J_INLINE_GETTER const node_t & front() const noexcept {
      return const_cast<avl_tree*>(this)->front();
    }

    [[nodiscard]] node_t & back() noexcept;
    J_INLINE_GETTER const node_t & back() const noexcept {
      return const_cast<avl_tree*>(this)->back();
    }

    [[nodiscard]] iterator before_begin() noexcept;
    J_INLINE_GETTER const_iterator before_begin() const noexcept {
      return const_cast<avl_tree*>(this)->before_begin();
    }

    [[nodiscard]] iterator begin() noexcept;
    J_INLINE_GETTER const_iterator begin() const noexcept {
      return const_cast<avl_tree*>(this)->begin();
    }

    [[nodiscard]] iterator end() noexcept;
    J_INLINE_GETTER const_iterator end() const noexcept {
      return const_cast<avl_tree*>(this)->end();
    }

    [[nodiscard]] iterator before_end() noexcept;
    J_INLINE_GETTER const_iterator before_end() const noexcept {
      return const_cast<avl_tree*>(this)->before_end();
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return !m_root;
    }

    void clear() noexcept;

    J_ALWAYS_INLINE ~avl_tree() {
      clear();
    }

    J_INLINE_GETTER u32_t depth() const noexcept {
      return m_root ? m_root->depth() : 0U;
    }

    J_INLINE_GETTER const node_t * root() const noexcept { return m_root; }
  private:
    node_t * m_root = nullptr;
  };
}

#define J_DECLARE_EXTERN_AVL_TREE(K, V)                                       \
  extern template struct j::containers::trees::binary_tree_node<K, V>;        \
  extern template struct j::containers::trees::binary_tree_iterator<          \
                          j::containers::trees::binary_tree_node<K, V>,       \
                          j::containers::comparer<K>>;                        \
  extern template struct j::containers::trees::binary_tree_iterator<          \
                          const j::containers::trees::binary_tree_node<K, V>, \
                          j::containers::comparer<K>>;                        \
  extern template class j::containers::trees::avl_tree<K, V>

#define J_DECLARE_EXTERN_AVL_TREE_NODE(K, V, N)                               \
  extern template struct j::containers::trees::binary_tree_iterator<          \
                          N, j::containers::comparer<K>>;                     \
  extern template struct j::containers::trees::binary_tree_iterator<          \
                          const N, j::containers::comparer<K>>;               \
  extern template class j::containers::trees::avl_tree<K, V, j::containers::comparer<K>, N>
