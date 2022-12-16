#pragma once

#include "containers/common.hpp"
#include "exceptions/assert_lite.hpp"
#include "algo/copy.hpp"
#include "hzd/mem.hpp"

namespace j::inline containers {
  template<typename T, u32_t MaxInline>
  struct inline_container final {
    J_INLINE_GETTER_NONNULL T * ptr(u32_t i) noexcept {
      J_ASSERT(i < MaxInline, "Out of range.");
      return (T*)(m_data + i * sizeof(T));
    }
    J_INLINE_GETTER_NONNULL const T * ptr(u32_t i) const noexcept {
      J_ASSERT(i < MaxInline, "Out of range.");
      return (const T*)(m_data + i * sizeof(T));
    }
    J_INLINE_GETTER_NONNULL const T * data() const noexcept
    { return (const T*)&m_data; }

    J_INLINE_GETTER_NONNULL T * data() noexcept
    { return (T*)&m_data; }

    alignas(T) u8_t m_data[MaxInline * sizeof(T)];
  };

  template<typename T>
  struct inline_container<T, 0U> final {};

  template<typename T, u32_t Size>
  struct stack_chunk final {
    J_INLINE_GETTER_NONNULL T * ptr(u32_t i) noexcept
    { return m_data.ptr(i); }
    J_INLINE_GETTER_NONNULL const T * ptr(u32_t i) const noexcept
    { return m_data.ptr(i); }

    J_INLINE_GETTER_NONNULL T * data() noexcept
    { return m_data.data(); }
    J_INLINE_GETTER_NONNULL const T * data() const noexcept
    { return m_data.data(); }

    stack_chunk() noexcept = default;
    explicit stack_chunk(stack_chunk * J_NOT_NULL next) noexcept
      : next(next)
    { }

    inline_container<T, Size> m_data;
    stack_chunk * next = nullptr;
  };

  template<typename T, u32_t MaxInline = 0U, u32_t NumPerChunk = 8U>
  class stack final {
  private:
    static constexpr inline u32_t num_per_ptr_v = sizeof(T*) / sizeof(T);
    static_assert(num_per_ptr_v <= NumPerChunk);
    using chunk_t = stack_chunk<T, NumPerChunk>;
  public:
    J_INLINE_GETTER T & back() noexcept {
      J_ASSUME(m_size != 0);
      return *ptr(m_size - 1U);
    }

    J_INLINE_GETTER const T & back() const noexcept {
      J_ASSUME(m_size != 0);
      return *ptr(m_size - 1U);
    }

    void pop_back() noexcept {
      J_ASSUME(m_size != 0);
      if constexpr (!is_trivially_copyable_v<T>) {
        ptr(m_size - 1U)->~T();
      }
      --m_size;
      if constexpr (MaxInline > 0 || num_per_ptr_v > 0) {
        if (m_size < MaxInline + num_per_ptr_v) {
          return;
        }
      }
      if constexpr (num_per_ptr_v > 0) {
        if (m_size == MaxInline + num_per_ptr_v) {
          chunk_t * chunk = m_data.ptr;
          algo::move_initialize_destroy(m_data.inline_data.data(), chunk->data(), num_per_ptr_v);
          ::delete chunk;
          return;
        }
      }
      if ((m_size - MaxInline) % NumPerChunk == 0U) {
        chunk_t * cur = m_data.ptr;
        m_data.ptr = cur->next;
        ::delete cur;
      }
    }

    template<typename... Args>
    T & emplace_back(Args && ... args) {
      T * result = nullptr;

      if constexpr (MaxInline > 0) {
        if (m_size < MaxInline) {
          result = m_inline.ptr(m_size);
          goto construct;
        }
      }

      if constexpr (num_per_ptr_v > 0) {
        if (m_size < MaxInline + num_per_ptr_v) {
          result = m_data.inline_data.ptr(m_size - MaxInline);
        } else if (m_size == MaxInline + num_per_ptr_v) {
          chunk_t * chunk = ::new chunk_t;
          algo::move_initialize_destroy(chunk->data(), m_data.inline_data.data(), num_per_ptr_v);
          if constexpr (num_per_ptr_v == NumPerChunk) {
            chunk = ::new chunk_t(chunk);
            result = chunk->data();
          } else {
            result = chunk->ptr(num_per_ptr_v);
          }
          m_data.ptr = chunk;
        }
        goto construct;
      }

      if ((m_size - MaxInline) % NumPerChunk == 0U) {
        chunk_t * const chunk = ::new chunk_t(m_data.ptr);
        m_data.ptr = chunk;
        result = chunk->ptr(0U);
      } else {
        result = m_data.ptr->ptr((m_size - MaxInline) % NumPerChunk);
      }

    construct:
      ++m_size;
      return *::new (result) T(static_cast<Args &&>(args)...);
    }

    J_ALWAYS_INLINE T & push_back(const T & t) { return emplace_back(t); }
    J_ALWAYS_INLINE T & push_back(T && t)      { return emplace_back(static_cast<T &&>(t)); }

    J_INLINE_GETTER u32_t size() const noexcept             { return m_size; }
    J_INLINE_GETTER bool empty() const noexcept             { return !m_size; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return m_size; }
    J_INLINE_GETTER bool operator!() const noexcept         { return !m_size; }

  private:
    [[nodiscard]] J_RETURNS_NONNULL T * ptr(u32_t i) noexcept {
      if constexpr (MaxInline > 0) {
        if (i < MaxInline) {
          return m_inline.ptr(i);
        }
      }
      if constexpr (num_per_ptr_v > 0) {
        if (m_size <= MaxInline + num_per_ptr_v) {
          return m_data.inline_data.ptr(i - MaxInline);
        }
      }
      J_ASSERT_NOT_NULL(m_data.ptr);
      return m_data.ptr->ptr((i - MaxInline) % NumPerChunk);
    }

    J_INLINE_GETTER_NONNULL const T * ptr(u32_t i) const noexcept
    { return const_cast<stack *>(this)->ptr(i); }

    u32_t m_size = 0U;
    [[no_unique_address]] inline_container<T, MaxInline> m_inline;
    union {
      chunk_t * ptr;
      inline_container<T, num_per_ptr_v> inline_data;
    } m_data = { nullptr };
  };
}
