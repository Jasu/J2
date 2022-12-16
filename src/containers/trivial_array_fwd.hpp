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
  class trivial_array {
  public:
    /// Construct an empty trivial array.
    J_A(AI,ND,NE,HIDDEN) inline trivial_array() noexcept = default;

    /// Move-construct the array.
    J_A(AI,ND,NE,HIDDEN) inline trivial_array(trivial_array && rhs) noexcept
      : m_data(rhs.m_data), m_size(rhs.m_size)
    { rhs.m_data = nullptr, rhs.m_size = 0U; }

    /// Move-construct elements from src into array.
    ///
    /// trivial_array does not take ownership of src, i.e. caller must free it.
    J_A(AI,ND,HIDDEN,NE) inline trivial_array(move_tag, T * src, u32_t size)
      : trivial_array(uninitialized, size)
    {
      if constexpr (is_trivially_copyable_v<T>) {
        m_size = size;
        ::j::memcpy(m_data, src, size * sizeof(T));
      } else {
        for (; m_size < size; ++m_size) {
          ::new (m_data + m_size) T(static_cast<T &&>(src[m_size]));
        }
      }
    }


    /// Move-construct elements from src into array.
    ///
    /// trivial_array does not take ownership of the elements, i.e. caller must free it.
    template<PtrRegion<T> U>
    J_A(AI,ND,HIDDEN,NE) inline trivial_array(move_tag, U & src)
      : trivial_array(move, src.begin(), src.size())
    { }

    /// Move-construct elements from an iterator range.
    ///
    /// trivial_array does not take ownership of the elements, i.e. caller must free it.
    template<typename It>
    trivial_array(move_tag, It first, const It last)
      : trivial_array(uninitialized, distance(first, last))
    {
      while (first != last) {
        initialize_element(static_cast<T &&>(*first++));
      }
    }

    /// Construct the array with uninitialized memory.
    ///
    /// The size of the array will be zero. The elements must be initialized with
    /// initialize_element().
    J_A(AI,ND,NE,HIDDEN) inline trivial_array(uninitialized_tag, u32_t size) noexcept
      : m_data(size ? reinterpret_cast<T*>(::j::allocate(size * sizeof(T))) : nullptr)
    { }


    /// Default-construct an array with size elements.
    explicit trivial_array(u32_t size);

    /// Move-initialize the next element in the array.
    ///
    /// The array must have been allocated with uninitialized_tag.
    T & initialize_element(T && element) noexcept;

    /// Emplace-initialize the next element in the array.
    template<typename... Args>
    J_A(AI,ND,HIDDEN,NE) inline T & initialize_element(Args && ... args)
      noexcept(is_nothrow_constructible_v<T, Args && ...>)
    {
      return *::new (m_data + m_size++) T(static_cast<Args &&>(args)...);
    }

    template<typename... Args>
    J_A(AI,HIDDEN,ND,NE) inline T & brace_initialize_element(Args && ... args) noexcept {
      return *::new (m_data + m_size++) T{static_cast<Args &&>(args)...};
    }

    /// Move elements from array to trivial_array.
    template<u32_t N>
    J_A(AI,ND,NE,HIDDEN) inline explicit trivial_array(move_tag, T (&& array)[N])
      : trivial_array(move, array, N)
    { }

    /// Move-assign to the array.
    trivial_array & operator=(trivial_array && rhs) noexcept;

    /// Returns the number of elements in the array.
    J_A(AI,NE,NODISC,ND,HIDDEN) inline u32_t size()             const noexcept { return m_size; }
    J_A(AI,NE,NODISC,ND,HIDDEN) inline bool empty()             const noexcept { return !m_size; }
    J_A(AI,NE,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept { return m_size; }
    J_A(AI,NE,NODISC,ND,HIDDEN) inline bool operator!()         const noexcept { return !m_size; }

    J_A(AI,NE,NODISC,ND,HIDDEN) inline T *       begin()        noexcept       { return m_data; }
    J_A(AI,NE,NODISC,ND,HIDDEN) inline const T * begin()        const noexcept { return m_data; }
    J_A(AI,NE,NODISC,ND,HIDDEN) inline T *       end()          noexcept       { return m_data + m_size; }
    J_A(AI,NE,NODISC,ND,HIDDEN) inline const T * end()          const noexcept { return m_data + m_size; }

    /// Access i:th element, without bounds checking.
    J_A(AI,NE,NODISC,ND,HIDDEN) inline T & operator[](u32_t i) noexcept {
      return m_data[i];
    }
    /// Access i:th element, without bounds checking.
    J_A(AI,NE,NODISC,ND,HIDDEN) inline const T & operator[](u32_t i) const noexcept {
      return m_data[i];
    }

    /// Access i:th element, with bounds checking.
    [[nodiscard]] inline T & at(u32_t i) {
      if (J_UNLIKELY(i >= m_size)) {
        exceptions::throw_out_of_range(0, i);
      }
      return m_data[i];
    }

    /// Access i:th element, with bounds checking.
    J_A(AI,NE,HIDDEN,NODISC) inline const T & at(u32_t i) const {
      return const_cast<trivial_array*>(this)->at(i);
    }

    /// Clear the array to have no elements.
    void clear() noexcept;

    J_A(AI,NE,HIDDEN) inline ~trivial_array() {
      clear();
    }

  protected:
    T * m_data = nullptr;
    u32_t m_size = 0U;
  };

  template<typename T>
  class trivial_array_copyable final : public trivial_array<T> {
  public:
    using trivial_array<T>::trivial_array;
    using trivial_array<T>::operator=;

    J_A(ND,NE,AI,HIDDEN) inline trivial_array_copyable() noexcept = default;

    /// Copy elements from array to trivial_array.
    template<u32_t N>
    J_A(AI,NE,ND,HIDDEN) inline trivial_array_copyable(copy_tag, const T (& array)[N])
      : trivial_array_copyable(copy, array, N) { }

    J_A(AI,NE,ND,HIDDEN) inline trivial_array_copyable(const trivial_array<T> & rhs)
      : trivial_array_copyable(copy, rhs.begin(), rhs.size())
    { }

    J_A(AI,NE,ND,HIDDEN) inline trivial_array_copyable(const trivial_array_copyable & rhs)
      : trivial_array_copyable(static_cast<const trivial_array<T> &>(rhs))
    { }

    J_A(AI,NE,HIDDEN,ND) inline trivial_array_copyable & operator=(const trivial_array<T> & rhs) {
      if (J_LIKELY(this != &rhs)) {
        clear();
        const u32_t sz = rhs.size();
        if (sz) {
          m_data = reinterpret_cast<T*>(::j::allocate(rhs.size() * sizeof(T)));
          if constexpr (is_trivially_copyable_v<T>) {
            ::j::memcpy(trivial_array<T>::m_data, rhs.begin(), sz * sizeof(T));
            m_size = sz;
          } else {
            for (; m_size != sz; ++m_size) {
              ::new (m_data + m_size) T(rhs[m_size]);
            }
          }
        }
      }
      return *this;
    }

    J_A(NE,AI,HIDDEN,ND) inline trivial_array_copyable & operator=(const trivial_array_copyable & rhs) {
      return operator=(static_cast<const trivial_array<T> &>(rhs));
    }

    /// Copy-construct elements from src into array.
    ///
    /// trivial_array does not take ownership of src, i.e. caller must free it.
    J_A(NE,AI,HIDDEN,ND) inline trivial_array_copyable(copy_tag, const T * src, u32_t size)
    : trivial_array<T>(uninitialized, size)
    {
      if constexpr (is_trivially_copyable_v<T>) {
        m_size = size;
        ::j::memcpy(m_data, src, size * sizeof(T));
      } else {
        if constexpr (is_nothrow_copy_constructible_v<T>) {
          for (; m_size < size; ++m_size) {
            ::new (m_data + m_size) T(src[m_size]);
          }
        } else {
          try {
            for (; m_size < size; ++m_size) {
              ::new (m_data + m_size) T(src[m_size]);
            }
          } catch (...) {
            clear();
            throw;
          }
        }
      }
    }

    /// Copy-construct elements from src into array.
    ///
    /// trivial_array does not take ownership of the elements, i.e. caller must free them.
    template<ConstPtrRegion<T> U>
    J_A(NE,AI,HIDDEN) inline trivial_array_copyable(copy_tag, U & src)
      : trivial_array_copyable(copy, src.begin(), src.size())
    { }

    using trivial_array<T>::clear;
  protected:
    using trivial_array<T>::m_data;
    using trivial_array<T>::m_size;
  };
}

#define J_DECLARE_EXTERN_TRIVIAL_ARRAY(T) extern template class j::trivial_array<T>
#define J_DECLARE_EXTERN_TRIVIAL_ARRAY_COPYABLE(T) extern template class j::trivial_array<T>; extern template class j::trivial_array_copyable<T>
