#pragma once

#include "mem/bump_pool.hpp"
#include "hzd/iterators.hpp"

namespace j::mem {
  template<typename T, typename Node, bool IsReverse>
  struct bump_pool_typed_iterator final {
    J_ALWAYS_INLINE bump_pool_typed_iterator() noexcept = default;
    explicit bump_pool_typed_iterator(bump_pool_iterator<Node, IsReverse> pool_it) noexcept
      : m_pool_it{pool_it}
    {
      if (m_pool_it) {
        if constexpr (IsReverse) {
          m_it = m_pool_it->template end<T>() - 1;
          m_end = m_pool_it->template begin<T>() - 1;
        } else {
          m_it = m_pool_it->template begin<T>();
          m_end = m_pool_it->template end<T>();
        }
        J_ASSUME_NOT_NULL(m_it, m_end);
      }
    }

    J_INLINE_GETTER_NO_DEBUG T & operator*() const noexcept {
      return *m_it;
    }

    J_INLINE_GETTER_NONNULL_NO_DEBUG T * operator->() const noexcept {
      return m_it;
    }

    bump_pool_typed_iterator & operator++() noexcept {
      J_ASSUME_NOT_NULL(m_it, m_end);
      if constexpr (IsReverse) {
        J_ASSUME(m_it > m_end);
        if (--m_it <= m_end) {
          if (!++m_pool_it) {
            m_end = m_it = nullptr;
          } else {
            m_it = m_pool_it->template end<T>() - 1;
            m_end = m_pool_it->template begin<T>() - 1;
            J_ASSUME_NOT_NULL(m_it, m_end);
            J_ASSUME(m_it >= m_end);
          }
        }
      } else {
        if (++m_it >= m_end) {
          if (!++m_pool_it) {
            m_end = m_it = nullptr;
          } else {
            m_it = m_pool_it->template begin<T>();
            m_end = m_pool_it->template end<T>();
            J_ASSUME_NOT_NULL(m_it, m_end);
            J_ASSUME(m_it <= m_end);
          }
        }
      }
      return *this;
    }

    bump_pool_typed_iterator operator++(int) noexcept {
      auto res{*this};
      return operator++(), res;
    }

    J_ALWAYS_INLINE bool operator==(const bump_pool_typed_iterator &) const noexcept = default;

    operator const bump_pool_typed_iterator<const T, const Node, IsReverse> &() const noexcept
    { return reinterpret_cast<const bump_pool_typed_iterator<const T, const Node, IsReverse>&>(*this); }
  private:
    T* m_it = nullptr;
    const T* m_end = nullptr;
    bump_pool_iterator<Node, IsReverse> m_pool_it;
  };

  template<typename T>
  class typed_bump_pool final {
  public:
    using iterator = bump_pool_typed_iterator<T, bump_pool_node, false>;
    using const_iterator = bump_pool_typed_iterator<const T, const bump_pool_node, false>;
    using reverse_iterator = bump_pool_typed_iterator<T, bump_pool_node, true>;
    using const_reverse_iterator = bump_pool_typed_iterator<const T, const bump_pool_node, true>;

    bump_pool_checkpoint checkpoint() const noexcept
    { return m_pool.checkpoint(); }

    void rewind_to_checkpoint(bump_pool_checkpoint point)
    { m_pool.rewind_to_checkpoint(point); }

    template<typename... Args>
    J_NO_DEBUG T & emplace(Args && ... args)
    { return m_pool.emplace_with_padding<T, Args...>(0, static_cast<Args &&>(args)...); }

    iterator begin() noexcept
    { return iterator(m_pool.begin()); }

    const_iterator begin() const noexcept
    { return const_iterator(m_pool.begin()); }

    iterator end() noexcept
    { return iterator(); }

    const_iterator end() const noexcept
    { return const_iterator(); }

    reverse_iterator rbegin() noexcept
    { return reverse_iterator(m_pool.rbegin()); }

    const_reverse_iterator rbegin() const noexcept
    { return const_iterator(m_pool.rbegin()); }

    reverse_iterator rend() noexcept
    { return reverse_iterator(); }

    const_reverse_iterator rend() const noexcept
    { return const_reverse_iterator(); }

    void clear() noexcept { m_pool.clear(); }

    auto reversed() noexcept { return reverse_view<typed_bump_pool>{this}; }
    auto reversed() const noexcept { return reverse_view<typed_bump_pool const>{this}; }
  private:
    bump_pool m_pool;
  };

}
