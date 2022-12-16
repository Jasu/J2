#pragma once

#include "exceptions/assert_lite.hpp"
#include "containers/common.hpp"
#include "hzd/type_traits.hpp"

namespace j::inline containers {
  template<typename T> struct strided_ptr {
    using ptr_t = conditional_t<is_const_v<T>, const char *, char*>;
    using vptr_t = conditional_t<is_const_v<T>, const void *, void*>;

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr() noexcept = default;

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr(null_t) noexcept { }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr(T * ptr) noexcept
      : ptr((ptr_t)ptr)
    { }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr(vptr_t ptr, i32_t stride) noexcept
      : ptr((ptr_t)ptr),
        stride(stride)
    {
      J_ASSERT_NOT_NULL(stride);
    }

    J_A(AI,ND,NODISC,HIDDEN) inline bool operator==(const strided_ptr & rhs) const noexcept {
      return ptr == rhs.ptr;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline bool operator<(const strided_ptr & rhs) const noexcept {
      return ptr < rhs.ptr;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline bool operator!() const noexcept { return !ptr; }
    J_A(AI,ND,NODISC,HIDDEN) inline explicit operator bool() const noexcept { return ptr; }

    J_A(AI,ND,NODISC,HIDDEN,RNN) inline T * operator->() const noexcept { return (T*)ptr; }
    J_A(AI,ND,NODISC,HIDDEN) inline T & operator*() const noexcept { return *(T*)ptr; }

    J_A(AI,ND,NODISC,HIDDEN) inline T & operator[](i32_t i) const noexcept {
      J_ASSERT_NOT_NULL(stride);
      return *(T*)(ptr + i * stride);
    }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr operator+(i32_t i) const noexcept {
      J_ASSERT_NOT_NULL(stride);
      return {ptr + i * stride, stride};
    }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr & operator++() noexcept {
      ptr += stride;
      return *this;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr operator++(int) noexcept {
      strided_ptr result{*this};
      ptr += stride;
      return result;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr & operator--() noexcept {
      ptr -= stride;
      return *this;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr operator--(int) noexcept {
      strided_ptr result{*this};
      ptr -= stride;
      return result;
    }

    J_A(AI,ND,NODISC,HIDDEN) inline strided_ptr operator-(i32_t i) const noexcept {
      return operator+(-i);
    }

    ptr_t ptr = nullptr;
    i32_t stride = 0;
  };

  template<typename T>
  J_A(AI,ND,NODISC,HIDDEN) inline i32_t operator-(const strided_ptr<T> & a, const strided_ptr<T> b) noexcept {
    i32_t stride = a.stride;
    if (!stride) {
      stride = b.stride;
      J_ASSERT_NOT_NULL(stride);
    } else {
      J_ASSERT(a.stride == b.stride || !b.stride, "Stride mismatch");
    }
    return (a.ptr - b.ptr) / a.stride;
  }


  template<typename T>
  struct strided_span {
    using ptr_t = conditional_t<is_const_v<T>, const char *, char*>;

    /// Construct an empty span.
    J_A(AI,ND,HIDDEN) inline strided_span() noexcept = default;

    J_A(AI,ND,HIDDEN) inline strided_span(T * J_AA(NN) data, i32_t size, i32_t stride) noexcept
      : m_data((ptr_t)data),
        m_size(size),
        m_stride(stride)
    {
      J_ASSERT(size >= 0 && stride);
    }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline i32_t size() const noexcept             { return m_size; }
    J_A(AI,NODISC,HIDDEN,ND,NE) inline i32_t stride() const noexcept           { return m_stride; }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline explicit operator bool() const noexcept { return m_size; }
    J_A(AI,NODISC,HIDDEN,ND,NE) inline bool operator!() const noexcept         { return !m_size; }

    J_A(AI,NODISC,HIDDEN,NE) inline T & operator[](i32_t idx) const noexcept {
      J_ASSERT_NOT_NULL(m_data, m_stride);
      J_ASSERT_RANGE(0, idx, m_size);
      return *(T*)(m_data + m_stride * idx);
    }

    inline void clear() noexcept { m_data = nullptr; m_size = 0; m_stride = 0; }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline strided_ptr<T> begin() const noexcept {
      return {m_data, m_stride};
    }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline strided_ptr<T> end() const noexcept {
      return {m_data + m_size * m_stride, m_stride };
    }

  private:
    ptr_t m_data = nullptr;
    i32_t m_size = 0;
    i32_t m_stride = 0;
  };

}
