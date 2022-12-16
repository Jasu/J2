#pragma once

#include "hzd/utility.hpp"

namespace j::rendering::vulkan::buffers::detail {
  template<typename Type, typename Ptr, u32_t Stride>
  class strided_iterator {
    Ptr m_ptr;
  public:
    J_ALWAYS_INLINE strided_iterator() noexcept = default;

    template<typename T, typename P, u32_t S>
    J_ALWAYS_INLINE strided_iterator(const strided_iterator<T, P, S> & rhs) noexcept
      : m_ptr(rhs.m_ptr)
    {
    }

    J_ALWAYS_INLINE explicit strided_iterator(Ptr ptr) noexcept : m_ptr(ptr) { }

    J_INLINE_GETTER bool operator==(const strided_iterator & rhs) const noexcept = default;

    J_INLINE_GETTER bool operator<(const strided_iterator & rhs) const noexcept {
      return m_ptr < rhs.m_ptr;
    }

    J_INLINE_GETTER bool operator>(const strided_iterator & rhs) const noexcept {
      return m_ptr > rhs.m_ptr;
    }

    J_INLINE_GETTER bool operator<=(const strided_iterator & rhs) const noexcept {
      return m_ptr <= rhs.m_ptr;
    }

    J_INLINE_GETTER bool operator>=(const strided_iterator & rhs) const noexcept {
      return m_ptr >= rhs.m_ptr;
    }

    strided_iterator & operator+=(i32_t rhs) const noexcept {
      m_ptr += Stride * rhs;
      return *this;
    }

    strided_iterator operator++(int) noexcept {
      strided_iterator res(m_ptr + Stride);
      ++this;
      return res;
    }

    strided_iterator & operator++() noexcept {
      m_ptr += Stride;
      return *this;
    }

    strided_iterator operator--(int) noexcept {
      strided_iterator res(m_ptr - Stride);
      --this;
      return res;
    }

    strided_iterator & operator-=(i32_t rhs) const noexcept {
      m_ptr -= Stride * rhs;
      return *this;
    }

    strided_iterator & operator--() noexcept {
      m_ptr -= Stride;
      return *this;
    }

    strided_iterator operator+(i32_t rhs) const noexcept {
      return m_ptr + Stride * rhs;
    }

    strided_iterator operator-(i32_t rhs) const noexcept {
      return m_ptr - Stride * rhs;
    }

    Type & operator*() noexcept {
      return *reinterpret_cast<Type*>(m_ptr);
    }

    const Type & operator*() const noexcept {
      return *reinterpret_cast<const Type*>(m_ptr);
    }

    J_INLINE_GETTER_NONNULL Type * operator->() noexcept {
      return reinterpret_cast<Type*>(m_ptr);
    }

    J_INLINE_GETTER_NONNULL const Type * operator->() const noexcept {
      return reinterpret_cast<const Type*>(m_ptr);
    }

    Type & operator[](i32_t rhs) noexcept {
      return *reinterpret_cast<Type*>(m_ptr + rhs * Stride);
    }

    const Type & operator[](i32_t rhs) const noexcept {
      return *reinterpret_cast<const Type*>(m_ptr + rhs * Stride);
    }

    i32_t operator-(const strided_iterator & rhs) const noexcept {
      return (m_ptr - rhs.m_ptr) / Stride;
    }
  };

  template<typename Type, u32_t Stride, typename Parent>
  class buffer_access : public Parent {
  public:
    using Parent::Parent;

    using iterator = detail::strided_iterator<Type, u8_t *, Stride>;
    using const_iterator = detail::strided_iterator<const Type, const u8_t *, Stride>;

    /// \return Size of the buffer, in number of elements.
    u32_t size() const noexcept;

    Type * data() noexcept;

    const Type * data() const noexcept;

    Type & operator[](u32_t index) noexcept;

    const Type & operator[](u32_t index) const noexcept;

    iterator begin() noexcept;

    const_iterator begin() const noexcept;

    iterator end() noexcept;

    const_iterator end() const noexcept;

    template<u32_t I>
    void assign(const Type (& data)[I]);

    template<u32_t I>
    void assign(Type (&& data)[I]) noexcept;

    template<u32_t I>
    void assign_at(u32_t offset, const Type (& data)[I]);

    template<u32_t I>
    void assign_at(u32_t offset, Type (&& data)[I]) noexcept;
  };

  template<typename T, u32_t S, typename P>
  u32_t buffer_access<T, S, P>::size() const noexcept {
    return (P::buffer_end() - P::buffer_begin()) / S;
  }

  template<typename T, u32_t S, typename P>
  T * buffer_access<T, S, P>::data() noexcept {
    return reinterpret_cast<T *>(P::buffer_begin());
  }

  template<typename T, u32_t S, typename P>
  const T * buffer_access<T, S, P>::data() const noexcept {
    return reinterpret_cast<const T *>(P::buffer_begin());
  }

  template<typename T, u32_t S, typename P>
  T & buffer_access<T, S, P>::operator[](u32_t index) noexcept {
    return *reinterpret_cast<T*>(P::buffer_begin() + index * S);
  }

  template<typename T, u32_t S, typename P>
  const T & buffer_access<T, S, P>::operator[](u32_t index) const noexcept {
    return *reinterpret_cast<const T*>(P::buffer_begin() + index * S);
  }

  template<typename T, u32_t S, typename P>
  typename buffer_access<T, S, P>::iterator buffer_access<T, S, P>::begin() noexcept {
    return iterator{ P::buffer_begin() };
  }

  template<typename T, u32_t S, typename P>
  typename buffer_access<T, S, P>::const_iterator buffer_access<T, S, P>::begin() const noexcept {
    return const_iterator{ P::buffer_begin() };
  }

  template<typename T, u32_t S, typename P>
  typename buffer_access<T, S, P>::iterator buffer_access<T, S, P>::end() noexcept {
    return iterator{ P::buffer_end() };
  }

  template<typename T, u32_t S, typename P>
  typename buffer_access<T, S, P>::const_iterator buffer_access<T, S, P>::end() const noexcept {
    return const_iterator{ P::buffer_end() };
  }

  template<typename T, u32_t S, typename P>
  template<u32_t I>
  void buffer_access<T, S, P>::assign(const T (& data)[I]) {
    for (u32_t i = 0; i < I; ++i) {
      (*this)[i] = data[i];
    }
  }

  template<typename T, u32_t S, typename P>
  template<u32_t I>
  void buffer_access<T, S, P>::assign(T (&& data)[I]) noexcept {
    for (u32_t i = 0; i < I; ++i) {
      (*this)[i] = static_cast<T &&>(data[i]);
    }
  }

  template<typename T, u32_t S, typename P>
  template<u32_t I>
  void buffer_access<T, S, P>::assign_at(u32_t offset, const T (& data)[I]) {
    for (u32_t i = 0; i < I; ++i) {
      (*this)[i + offset] = data[i];
    }
  }

  template<typename T, u32_t S, typename P>
  template<u32_t I>
  void buffer_access<T, S, P>::assign_at(u32_t offset, T (&& data)[I]) noexcept {
    for (u32_t i = 0; i < I; ++i) {
      (*this)[i + offset] = static_cast<T &&>(data[i]);
    }
  }
}
