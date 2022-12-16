#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::inline containers {
  template<typename T, i32_t Size>
  struct static_vector {
    J_A(AI,NE,ND,HIDDEN) inline static_vector() noexcept { }

    J_A(AI,NE,ND,HIDDEN) inline static_vector(static_vector && rhs) noexcept
      : sz(rhs.sz)
    {
      for (T *to = (T*)buf, *from = (T*)rhs.buf, *end = to + sz; to != end; ++to, ++from) {
        ::new (to) T((T&&)*from);
      }
      rhs.sz = 0;
    }

    J_A(ND) inline static_vector(const static_vector & rhs)
      : sz(rhs.sz)
    {
      const T * from = (const T*)rhs.buf;
      for (T *to = (T*)buf, *end = to + sz; to != end; ++to, ++from) {
        ::new (to) T(*from);
      }
    }

    J_A(AI,NE,ND,HIDDEN) static_vector & operator=(static_vector && rhs) noexcept {
      if (this != &rhs) {
        clear();
        sz = rhs.sz;
        for (T *to = (T*)buf, *from = (T*)rhs.buf, *end = to + sz; to != end; ++to, ++from) {
          ::new (to) T((T&&)*from);
        }
        rhs.sz = 0;
      }
      return *this;
    }

    J_A(ND) inline static_vector & operator=(const static_vector & rhs) {
      if (this != &rhs) {
        clear();
        sz = rhs.sz;
        const T * from = (const T*)rhs.buf;
        for (T *to = (T*)buf, *end = to + sz; to != end; ++to, ++from) {
          ::new (to) T(*from);
        }
      }
      return *this;
    }

    J_A(AI) inline ~static_vector() {
      clear();
    }

    inline void pop_back() noexcept {
      J_ASSERT(sz);
      ((T*)buf)[--sz].~T();
    }

    inline void clear() noexcept {
      for (i32_t i = sz - 1; i >= 0; --i) {
        ((T*)buf)[i].~T();
      }
      sz = 0;
    }

    template<typename... U>
    J_A(AI,HIDDEN,NE,RNN) inline T * emplace_back(U && ... us) {
      J_ASSERT(sz < Size);
      return ::new ((T*)buf + sz++) T((U&&)us...);
    }

    J_A(RNN) inline T * push_back(T && t) noexcept {
      J_ASSERT(sz < Size);
      return ::new ((T*)buf + sz++) T((T&&)t);
    }

    J_A(RNN) inline T * push_back(const T & t) {
      J_ASSERT(sz < Size);
      return ::new ((T*)buf + sz++) T(t);
    }

    inline void resize(i32_t n) {
      J_ASSERT_RANGE(0, n, Size);
      if (n < sz) {
        do {
          ((T*)buf)[--sz].~T();
        } while (n < sz);
      } else {
        while (n > sz) {
          emplace_back();
        }
      }
    }

    J_A(AI,HIDDEN,NE,NODISC) inline T & operator[](i32_t i) noexcept {
      return ((T*)buf)[i];
    }

    J_A(AI,HIDDEN,NE,NODISC) inline const T & operator[](i32_t i) const noexcept {
      return ((const T*)buf)[i];
    }

    J_A(AI,NE,ND,NODISC,HIDDEN,RNN) inline T * begin() noexcept {
      return (T*)buf;
    }

    J_A(AI,NE,ND,NODISC,HIDDEN,RNN) inline const T * begin() const noexcept {
      return (const T*)buf;
    }

    J_A(AI,NE,ND,NODISC,HIDDEN,RNN) inline T * end() noexcept {
      return (T*)buf + sz;
    }

    J_A(AI,NE,ND,NODISC,HIDDEN,RNN) inline const T * end() const noexcept {
      return (const T*)buf + sz;
    }

    J_A(AI,NE,ND,NODISC,HIDDEN) inline T & back() noexcept {
      J_ASSERT(sz);
      return ((T*)buf)[sz-1];
    }

    J_A(AI,NE,ND,NODISC,HIDDEN) inline const T & back() const noexcept {
      J_ASSERT(sz);
      return ((const T*)buf)[sz-1];
    }

    J_A(AI,NE,ND,NODISC,HIDDEN) inline i32_t size()             const noexcept { return sz; }
    J_A(AI,NE,ND,NODISC,HIDDEN) inline bool empty()             const noexcept { return !sz; }
    J_A(AI,NE,ND,NODISC,HIDDEN) inline bool operator!()         const noexcept { return !sz; }
    J_A(AI,NE,ND,NODISC,HIDDEN) inline explicit operator bool() const noexcept { return sz; }

    i32_t sz = 0;
    alignas(T) char buf[Size * sizeof(T)];
  };
}
