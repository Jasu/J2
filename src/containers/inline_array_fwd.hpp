#pragma once

#include "containers/common.hpp"
#include "hzd/mem.hpp"
#include "hzd/iterators.hpp"
#include "hzd/string.hpp"
#include "hzd/concepts.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::inline containers {
  /// Move-only statically-sized array.
  template<typename T>
  class alignas(T) inline_array {
  public:
    /// Construct an empty trivial array.
    J_ALWAYS_INLINE J_NO_EXPLICIT constexpr inline_array() noexcept = default;

    /// Move-construct elements from src into array.
    ///
    /// trivial_array does not take ownership of src, i.e. caller must free it.
    template<typename Tag, typename Ptr>
    J_NO_EXPLICIT inline_array(Tag, Ptr * src, const u32_t size)
      : m_size(size)
    {
      if constexpr (is_trivially_copyable_v<T>) {
        ::j::memcpy(this + 1, src, size * sizeof(T));
      } else {
        T * cur = begin(), * const e = end();
        while (cur != e) {
          if constexpr (is_same_v<Tag, move_tag>) {
            ::new (cur) T(static_cast<T &&>(*src));
          } else {
            ::new (cur) T(*src);
          }
          ++src;
          ++cur;
        }
      }
    }

    /// Move-construct the array.
    J_ALWAYS_INLINE J_NO_EXPLICIT constexpr inline_array(inline_array && rhs) noexcept
      : inline_array(move, rhs.begin(), rhs.m_size)
    {
      rhs.m_size = 0U;
    }

    /// Copy-construct the array.
    J_ALWAYS_INLINE J_NO_EXPLICIT constexpr inline_array(const inline_array & rhs) noexcept
      : inline_array(copy, rhs.begin(), rhs.m_size)
    { }

    /// Move-construct elements from src into array.
    ///
    /// inline_array does not take ownership of the elements, i.e. caller must free it.
    template<PtrRegion<T> U>
    J_NO_EXPLICIT J_ALWAYS_INLINE inline_array(move_tag, U & src) noexcept
      : inline_array(move, src.begin(), src.size())
    { }

    /// Copy-construct elements from src into array.
    ///
    /// inline_array does not take ownership of the elements, i.e. caller must free it.
    template<ConstPtrRegion<T> U>
    J_NO_EXPLICIT J_ALWAYS_INLINE inline_array(const U & src) noexcept(is_nothrow_copy_constructible_v<T>)
      : inline_array(copy, src.begin(), src.size())
    { }

    /// Move-construct elements from an iterator range.
    ///
    /// trivial_array does not take ownership of the elements, i.e. caller must free it.
    template<typename It>
    inline_array(move_tag, It first, const It last) noexcept
      : m_size(distance(first, last))
    {
      T * cur = begin();
      while (first != last) {
        ::new (cur++) T(static_cast<T &&>(*first++));
      }
    }

    /// Move-construct elements from an iterator range.
    ///
    /// trivial_array does not take ownership of the elements, i.e. caller must free it.
    template<typename It>
    inline_array(copy_tag, It first, const It last) noexcept(is_nothrow_copy_constructible_v<T>)
      : m_size(distance(first, last))
    {
      T * cur = begin();
      while (first != last) {
        ::new (cur++) T(*first++);
      }
    }

    /// Construct the array with uninitialized memory.
    ///
    /// The size of the array will be zero. The elements must be initialized with
    /// initialize_element().
    J_ALWAYS_INLINE J_NO_EXPLICIT inline_array(uninitialized_tag, u32_t size) noexcept
      : m_size(size)
    {
    }

    /// Default-construct an array with size elements.
    explicit inline_array(u32_t size) noexcept(is_nothrow_constructible_v<T>);

    /// Move elements from array to trivial_array.
    template<u32_t N>
    J_ALWAYS_INLINE explicit inline_array(move_tag, T (&& array)[N])
      : inline_array(move, array, N)
    { }

    /// Move-assign to the array.
    inline_array & operator=(inline_array && rhs) = delete;

    /// Returns the number of elements in the array.
    J_INLINE_GETTER J_NO_EXPLICIT u32_t size()             const noexcept { return m_size; }
    J_INLINE_GETTER J_NO_EXPLICIT bool empty()             const noexcept { return !m_size; }
    J_ALWAYS_INLINE J_NO_EXPLICIT explicit operator bool() const noexcept { return m_size; }
    J_ALWAYS_INLINE J_NO_EXPLICIT bool operator!()         const noexcept { return !m_size; }

    J_INLINE_GETTER J_NO_EXPLICIT T *       begin()        noexcept       { return reinterpret_cast<T*>(this + 1); }
    J_INLINE_GETTER J_NO_EXPLICIT const T * begin()        const noexcept { return reinterpret_cast<const T*>(this + 1); }
    J_INLINE_GETTER J_NO_EXPLICIT T *       end()          noexcept       { return reinterpret_cast<T*>(this + 1) + m_size; }
    J_INLINE_GETTER J_NO_EXPLICIT const T * end()          const noexcept { return reinterpret_cast<const T*>(this + 1) + m_size; }

    /// Access i:th element, without bounds checking.
    J_INLINE_GETTER J_NO_EXPLICIT T & operator[](u32_t i) noexcept {
      J_ASSUME(i < m_size);
      return begin()[i];
    }
    /// Access i:th element, without bounds checking.
    J_INLINE_GETTER J_NO_EXPLICIT const T & operator[](u32_t i) const noexcept {
      J_ASSUME(i < m_size);
      return begin()[i];
    }

    /// Access i:th element, with bounds checking.
    [[nodiscard]] J_NO_EXPLICIT J_HIDDEN T & at(u32_t i);
    /// Access i:th element, with bounds checking.
    J_INLINE_GETTER J_NO_EXPLICIT const T & at(u32_t i) const
    { return const_cast<inline_array*>(this)->at(i); }

    /// Clear the array to have no elements.
    void clear() noexcept;

    J_ALWAYS_INLINE J_NO_EXPLICIT ~inline_array() {
      if constexpr (!is_trivially_destructible_v<T>) {
        clear();
      }
    }

    static constexpr J_ALWAYS_INLINE J_NO_EXPLICIT u32_t get_byte_size(u32_t size) noexcept {
      return sizeof(inline_array) + size * sizeof(T);
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT T & pop_back() noexcept {
      J_ASSUME(m_size);
      return begin()[--m_size];
    }

  protected:
    u32_t m_size = 0U;
  };
}

#define J_DECLARE_EXTERN_INLINE_ARRAY(T) extern template class j::containers::inline_array<T>
