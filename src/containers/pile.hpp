#pragma once

#include "containers/sbo.hpp"
#include "exceptions/assert_lite.hpp"
#include "hzd/mem.hpp"
#include "hzd/string.hpp"

namespace j::inline containers {
  template<typename T, u32_t NumInline = 4U>
  struct pile_iterator final {
    T *it, *inline_end, *data_begin;

    J_INLINE_GETTER T * operator->() const noexcept {
      return it;
    }

    J_INLINE_GETTER T & operator*() const noexcept {
      return *it;
    }

    J_ALWAYS_INLINE pile_iterator & operator++() noexcept {
      ++it;
      if (it == inline_end) {
        it = data_begin;
      }
      return *this;
    }

    J_ALWAYS_INLINE pile_iterator operator++(int) noexcept {
      pile_iterator result{*this};
      return operator++(), result;
    }

    J_INLINE_GETTER bool operator==(const pile_iterator & rhs) const noexcept
    { return it == rhs.it; }

    J_INLINE_GETTER operator const pile_iterator<const T, NumInline> & () const noexcept
    { return reinterpret_cast<const pile_iterator<const T, NumInline> &>(*this); }
  };

  template<typename T, u32_t NumInline = 4U>
  class pile final {
  public:
    using iterator = pile_iterator<T, NumInline>;
    using const_iterator = pile_iterator<const T, NumInline>;

    J_ALWAYS_INLINE constexpr pile() noexcept = default;

    pile(pile && rhs) noexcept
      : m_size(rhs.m_size),
        m_capacity(rhs.m_capacity),
        m_data(rhs.m_data)
    {
      move_inline_data_from(rhs);
      rhs.m_size = 0U;
      rhs.m_data = nullptr;
      rhs.m_capacity = NumInline;
    }

    pile & operator=(pile && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        clear();
        m_size = rhs.m_size;
        m_capacity = rhs.m_capacity;
        m_data = rhs.m_data;
        move_inline_data_from(rhs);
        rhs.m_size = 0U;
        rhs.m_data = nullptr;
        rhs.m_capacity = NumInline;
      }
      return *this;
    }

    template<typename... Args>
    J_ALWAYS_INLINE T & emplace_back(Args && ... args) {
      maybe_grow();
      return *::new (ptr(m_size++)) T(static_cast<Args &&>(args)...);
    }

    template<typename... Args>
    T & emplace_front(Args && ... args) {
      maybe_grow();
      if (m_size) {
        ::new (ptr(m_size++)) T(static_cast<T &&>(*ptr(0)));
        return *ptr(0) = T(static_cast<Args &&>(args)...);
      } else {
        ++m_size;
        return *::new (ptr(0)) T(static_cast<Args &&>(args)...);
      }
    }

    J_ALWAYS_INLINE void pop_back() noexcept {
      J_ASSERT(m_size > 0);
      ptr(m_size--)->~T();
    }

    inline iterator erase(iterator it) noexcept {
      J_ASSERT(it.it && m_size > 0);
      T * const last = ptr(--m_size);
      *it = static_cast<T &&>(*last);
      last->~T();
      return it;
    }

    J_INLINE_GETTER T & operator[](u32_t i) noexcept {
      return *ptr(i);
    }

    J_INLINE_GETTER const T & operator[](u32_t i) const noexcept {
      return *ptr(i);
    }

    J_INLINE_GETTER u32_t size() const noexcept { return m_size; }

    J_INLINE_GETTER bool empty() const noexcept { return !m_size; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return m_size; }
    J_INLINE_GETTER bool operator!() const noexcept { return !m_size; }

    J_INLINE_GETTER iterator begin() noexcept {
      if constexpr (NumInline > 0) {
        return iterator{m_inline.data(), m_inline.data() + NumInline, m_data};
      } else {
        return iterator{m_data, nullptr, nullptr};
      }
    }

    J_INLINE_GETTER iterator end() noexcept {
      if constexpr (NumInline > 0) {
        return iterator{ptr(m_size), m_inline.data() + NumInline, m_data};
      } else {
        return iterator{ptr(m_size), nullptr, nullptr};
      }
    }

    J_INLINE_GETTER const_iterator begin() const noexcept {
      if constexpr (NumInline > 0) {
        return const_iterator{m_inline.data(), m_inline.data() + NumInline, m_data};
      } else {
        return const_iterator{m_data, nullptr, nullptr};
      }
    }

    J_INLINE_GETTER const_iterator end() const noexcept {
      if constexpr (NumInline > 0) {
        return const_iterator{ptr(m_size), m_inline.data() + NumInline, m_data};
      } else {
        return const_iterator{ptr(m_size), nullptr, nullptr};
      }
    }

    void clear() noexcept {
      if constexpr (!is_trivially_destructible_v<T>) {
        u32_t i = 0U;
        for (const u32_t n = j::min(m_size, NumInline); i < n; ++i) {
          m_inline.ptr(i)->~T();
        }
        for (const u32_t n = m_size; i < n; ++i) {
          m_data[i - NumInline].~T();
        }
      }
      m_size = 0U;
      if (m_capacity > NumInline) {
        j::free(m_data);
        m_data = nullptr;
        m_capacity = NumInline;
      }
    }

    J_ALWAYS_INLINE ~pile() { clear(); }
  private:
    void maybe_grow() {
      if (m_size < m_capacity) {
        return;
      }
      if (m_capacity == NumInline) {
        m_data = j::allocate<T>(8U);
      } else {
        if constexpr (is_nicely_copyable_v<T>) {
          m_data = j::reallocate<T>(m_data, m_capacity + 8U - NumInline);
        } else {
          const auto d = j::allocate<T>(m_capacity + 8U - NumInline);
          for (u32_t i = 0U, end = m_size - NumInline; i < end; ++i) {
            ::new (d + i) T(static_cast<T &&>(m_data[i]));
            m_data[i].~T();
          }
          j::free(m_data);
          m_data = d;
        }
      }
      m_capacity += 8U;
    }

    [[nodiscard]] T * ptr(u32_t index) const noexcept {
      if (index >= NumInline) {
        return m_data + (index - NumInline);
      } else {
        return m_inline.ptr(index);
      }
    }

    void move_inline_data_from(pile & rhs) noexcept {
      if constexpr (is_nicely_copyable_v<T>) {
        ::j::memcpy(m_inline.data(), rhs.m_inline.data(), NumInline * sizeof(T));
      } else {
        for (u32_t i = 0U, end = j::max(NumInline, m_size); i < end; ++i) {
          const auto src = rhs.m_inline.ptr(i);
          ::new (m_inline.ptr(i)) T(static_cast<T &&>(*src));
          src->~T();
        }
      }
    }

    u32_t m_size = 0U;
    u32_t m_capacity = NumInline;
    [[no_unique_address]] inline_wrapper<T, NumInline> m_inline;
    T * m_data = nullptr;
  };
}
