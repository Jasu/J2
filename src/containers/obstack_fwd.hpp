#pragma once

#include "hzd/utility.hpp"

namespace j::detail {
  struct obstack_chunk;
}

namespace j::inline containers {
  template<typename T, typename Chunk>
  class obstack_iterator {
  public:
    T & operator*() const noexcept;

    T * operator->() const noexcept J_RETURNS_NONNULL;

    obstack_iterator & operator++() noexcept;

    obstack_iterator operator++(int) noexcept;

    J_INLINE_GETTER operator obstack_iterator<const T, const Chunk>() const noexcept
    { return { chunk, i }; }

    J_INLINE_GETTER J_NO_EXPLICIT bool operator==(const obstack_iterator &) const noexcept = default;

    Chunk * chunk;
    u32_t i;
  };

  template<typename T, int NumPerChunk = 8>
  class obstack {
  public:
    using iterator J_NO_DEBUG_TYPE = obstack_iterator<T, detail::obstack_chunk>;
    using const_iterator J_NO_DEBUG_TYPE = obstack_iterator<const T, const detail::obstack_chunk>;

    J_NO_EXPLICIT J_ALWAYS_INLINE constexpr obstack() noexcept = default;

    J_NO_EXPLICIT J_HIDDEN obstack(const obstack & rhs) {
      try {
        copy_initialize(rhs.m_head);
      } catch (...) {
        clear();
        throw;
      }
    }

    J_NO_EXPLICIT J_ALWAYS_INLINE obstack(obstack && rhs) noexcept
      : m_head(rhs.m_head)
    { rhs.m_head = nullptr; }

    J_NO_EXPLICIT J_ALWAYS_INLINE ~obstack() {
      clear();
    }


    J_ALWAYS_INLINE J_NO_EXPLICIT obstack & operator=(const obstack & rhs) {
      if (this != &rhs) {
        clear();
        copy_initialize(rhs.m_head);
      }
      return *this;
    }

    obstack & operator=(obstack && rhs) noexcept;

    T & push(const T & t);
    T & push(T && t);

    template<typename... Args>
    T & emplace(Args && ... args);

    u32_t size() const noexcept;
    J_INLINE_GETTER J_NO_EXPLICIT bool empty() const noexcept
    { return !m_head; }

    T & top();
    J_INLINE_GETTER J_NO_EXPLICIT const T & top() const
    { return const_cast<obstack *>(this)->top(); }

    void pop() noexcept;

    void clear() noexcept;

    J_INLINE_GETTER J_NO_EXPLICIT iterator begin() noexcept {
      return iterator{ m_head, 0 };
    }

    J_INLINE_GETTER J_NO_EXPLICIT const_iterator begin() const noexcept {
      return const_iterator{ m_head, 0 };
    }

    J_INLINE_GETTER J_NO_EXPLICIT constexpr iterator end() noexcept {
      return iterator{ nullptr, 0 };
    }

    J_INLINE_GETTER J_NO_EXPLICIT constexpr const_iterator end() const noexcept {
      return const_iterator{ nullptr, 0 };
    }
  private:
    J_NO_EXPLICIT void copy_initialize(detail::obstack_chunk * c);
    detail::obstack_chunk * m_head = nullptr;
  };
}
#define J_DECLARE_EXTERN_OBSTACK(TYPE)                                                               \
  extern template class j::containers::obstack<TYPE>;                                                \
  extern template class j::containers::obstack_iterator<TYPE, j::detail::obstack_chunk>; \
  extern template class j::containers::obstack_iterator<const TYPE, const j::detail::obstack_chunk>
