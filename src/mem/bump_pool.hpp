#pragma once

#include "strings/string_view.hpp"
#include "mem/bump_pool_fwd.hpp"

namespace j::mem {
  struct bump_pool_node final {
    bump_pool_node(u16_t size, u32_t size_before, bump_pool_node * next) noexcept;
    u16_t size = 0U;
    /// Size left for user data.
    u16_t size_left = 0U;
    /// Size allocated before this node.
    u32_t size_before = 0U;
    bump_pool_node * next = nullptr;
    bump_pool_node * previous = nullptr;

    template<typename T>
    J_A(AI,NODISC,RNN) inline T * begin() noexcept J_RETURNS_ALIGNED(16)
    { return reinterpret_cast<T*>(this + 1U); }

    template<typename T>
    J_A(AI,NODISC,RNN) inline const T * begin() const noexcept J_RETURNS_ALIGNED(16)
    { return reinterpret_cast<const T*>(this + 1U); }

    template<typename T>
    J_A(AI,NODISC,RNN) inline T * end() noexcept
    { return add_bytes<T*>(this + 1U, size); }

    template<typename T>
    J_A(AI,NODISC,RNN) inline const T * end() const noexcept
    { return add_bytes<T*>(this + 1U, size); }
  };

  /// Iterates the nodes in a bump pool, from oldest to newest.
  template<typename Node, bool IsReverse>
  struct bump_pool_iterator final {
    inline bump_pool_iterator & operator++() noexcept {
      if (IsReverse) {
        m_node = m_node->next;
      } else {
        m_node = m_node->previous;
      }
      return *this;
    }

    inline bump_pool_iterator operator++(int) noexcept {
      bump_pool_iterator res{m_node};
      operator++();
      return res;
    }

    J_ALWAYS_INLINE bool operator==(const bump_pool_iterator &) const noexcept = default;

    [[nodiscard]] inline operator bump_pool_iterator<const bump_pool_node, IsReverse>() const noexcept {
      return {m_node};
    }

    J_INLINE_GETTER Node & operator*() const noexcept J_RETURNS_PAGE_ALIGNED {
      return *m_node;
    }

    J_INLINE_GETTER_NONNULL Node * operator->() const noexcept J_RETURNS_PAGE_ALIGNED {
      return m_node;
    }

    J_INLINE_GETTER bool operator!() const noexcept { return !m_node; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return m_node; }

    inline void reset() noexcept {
      m_node = nullptr;
    }

    Node * m_node = nullptr;
  };

  class bump_pool final {
  public:
    bump_pool() noexcept;

    using iterator = bump_pool_iterator<bump_pool_node, false>;
    using const_iterator = bump_pool_iterator<const bump_pool_node, false>;
    using reverse_iterator = bump_pool_iterator<bump_pool_node, true>;
    using const_reverse_iterator = bump_pool_iterator<const bump_pool_node, true>;

    [[nodiscard]] const void * ptr_of(u32_t index) const noexcept;
    [[nodiscard]] u32_t index_of(const void * J_NOT_NULL ptr) const noexcept;

    [[nodiscard]] inline void * current() const noexcept
    { return m_top ? m_top->end<void>() : nullptr; }

    [[nodiscard]] J_RETURNS_NONNULL void * reserve(u32_t sz);

    J_RETURNS_NONNULL void * allocate(u32_t sz);
    J_RETURNS_NONNULL void * allocate_aligned_zero(u32_t sz, u32_t align);

    void align(u32_t sz);

    [[nodiscard]] bump_pool_checkpoint checkpoint() const noexcept;

    void rewind_to_checkpoint(bump_pool_checkpoint point);

    template<typename T, typename... Args>
    T & emplace_with_padding(u32_t padding, Args && ... args) {
      void * const ptr = allocate(sizeof(T) + padding);
      return *::new (ptr) T(static_cast<Args &&>(args)...);
    }

    template<typename T, typename... Args>
    T & emplace_with_padding_braces(u32_t padding, Args && ... args) {
      void * const ptr = allocate(sizeof(T) + padding);
      return *::new (ptr) T{static_cast<Args &&>(args)...};
    }

    template<typename T, typename... Args>
    T & emplace(Args && ... args)
    { return emplace_with_padding<T, Args...>(0, static_cast<Args &&>(args)...); }

    template<typename T, typename... Args>
    T & emplace_braces(Args && ... args)
    { return emplace_with_padding_braces<T, Args...>(0, static_cast<Args &&>(args)...); }

    strings::const_string_view write_string(strings::const_string_view str);

    [[nodiscard]] inline u32_t size() const noexcept
    { return m_top ? m_top->size_before + m_top->size : 0U; }

    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] inline iterator begin() noexcept
    { return iterator{m_bottom}; }

    [[nodiscard]] inline const_iterator begin() const noexcept
    { return const_iterator{m_bottom}; }

    [[nodiscard]] inline iterator end() noexcept
    { return iterator(); }

    [[nodiscard]] inline const_iterator end() const noexcept
    { return const_iterator(); }

    [[nodiscard]] reverse_iterator rbegin() noexcept
    { return reverse_iterator{m_top}; }

    [[nodiscard]] const_reverse_iterator rbegin() const noexcept
    { return const_reverse_iterator{m_top}; }

    [[nodiscard]] reverse_iterator rend() noexcept
    { return reverse_iterator(); }

    [[nodiscard]] const_reverse_iterator rend() const noexcept
    { return const_reverse_iterator(); }

    void clear() noexcept;

    void pop_nodes_until(bump_pool_node * node) noexcept;

    ~bump_pool();

    [[nodiscard]] inline u32_t size_available() const noexcept
    { return m_top ? m_top->size_left : 0U; }

  private:

    bump_pool_node * m_bottom = nullptr;
    bump_pool_node * m_top = nullptr;
  };
}
