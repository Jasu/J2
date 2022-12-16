#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/concepts.hpp"
#include "containers/common.hpp"

namespace j::inline containers {
  template<typename T>
  class span {
  public:
    /// Construct an empty span.
    J_A(AI,ND,HIDDEN) constexpr inline span() noexcept = default;

    template<u32_t N>
    J_A(AI,ND,HIDDEN) inline span(T (& data)[N]) noexcept : m_data(data), m_size(N) { }
    template<u32_t N>
    J_A(AI,ND,HIDDEN) inline span(T (&& data)[N]) noexcept : m_data(data), m_size(N) { }

    J_A(AI,ND,HIDDEN) inline span(T * data, i32_t size) noexcept
      : m_data(data), m_size(size)
    { }

    J_A(AI,ND,HIDDEN) inline span(T * J_NOT_NULL begin, T * J_NOT_NULL end) noexcept
      : m_data(begin), m_size(end - begin)
    { }

    template<PtrRegion<T> U>
    J_A(ND,AI,HIDDEN) inline span(U && iterable) noexcept
      : m_data(iterable.begin()),
        m_size(iterable.size())
    { }

    template<ConstPtrRegion<T> U>
    J_A(ND,AI,HIDDEN) inline span(const U & iterable) noexcept
      : m_data(iterable.begin()),
        m_size(iterable.size())
    { }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline u32_t size() const noexcept             { return m_size; }
    J_A(ND) inline u32_t size_bytes() const noexcept       { return m_size * sizeof(T); }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline bool empty() const noexcept             { return !m_size; }
    J_A(AI,NODISC,HIDDEN,ND,NE) inline explicit operator bool() const noexcept { return m_size; }
    J_A(AI,NODISC,HIDDEN,ND,NE) inline bool operator!() const noexcept         { return !m_size; }

    J_A(AI,NODISC,HIDDEN,NE) inline T & operator[](u32_t idx) const noexcept {
      return m_data[idx];
    }

    inline void clear() noexcept { m_data = nullptr; m_size = 0U; }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline T * begin() const noexcept { return m_data; }
    J_A(AI,NODISC,HIDDEN,ND,NE) inline T * end() const noexcept   { return m_data + m_size; }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline T & front() const noexcept {
      J_ASSERT_SZ(m_size);
      return *m_data;
    }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline T & back() const noexcept {
      J_ASSERT_SZ(m_size);
      return m_data[m_size - 1U];
    }

    J_A(AI,NODISC,HIDDEN,ND,NE) inline bool operator==(const span & rhs) const noexcept = default;

    [[nodiscard]] T & at(i32_t idx) const {
      if (J_UNLIKELY((u32_t)idx >= (u32_t)m_size)) {
        exceptions::throw_out_of_range_0(idx, m_size);
      }
      return m_data[idx];
    }

    inline T & pop_front() noexcept {
      J_ASSERT_SZ(m_size);
      return --m_size, *m_data++;
    }

    inline T & pop_back() noexcept {
      J_ASSERT_SZ(m_size);
      return m_data[--m_size];
    }

    [[nodiscard]] span slice(i32_t start, i32_t len) noexcept {
      J_ASSERT((u32_t)start <= (u32_t)m_size && (u32_t)len <= (u32_t)(m_size - start));
      return span(m_data + start, len);
    }

    [[nodiscard]] span prefix(u32_t num) const noexcept {
      J_ASSERT_RANGE_LTE(num, m_size);
      return span(m_data, num);
    }

    J_A(AI,ND,NODISC,HIDDEN) inline span suffix(i32_t num) const noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      return span(m_data + num, num);
    }

    J_A(AI,ND,NODISC,HIDDEN) inline span without_prefix(i32_t num) const noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      return span(m_data + num, m_size - num);
    }

    J_A(AI,ND,NODISC,HIDDEN) inline span without_suffix(i32_t num) const noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      return span(m_data, m_size - num);
    }

    J_A(AI,ND,HIDDEN) inline void remove_prefix(i32_t num) noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      m_data += num;
      m_size -= num;
    }

    J_A(AI,ND,HIDDEN) inline void remove_prefix(T * J_NOT_NULL it) noexcept {
      remove_prefix(it - m_data);
    }

    J_A(AI,ND,HIDDEN) inline void remove_suffix(i32_t num) noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      m_size -= num;
    }

    [[nodiscard]] span take_prefix(i32_t num) noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      const span prefix(m_data, num);
      return m_data += num, m_size -= num, prefix;
    }

    [[nodiscard]] span take_suffix(i32_t num) noexcept {
      J_ASSERT_RANGE_0_LTE(num, m_size);
      m_size -= num;
      return span(m_data + m_size, num);
    }

    J_A(AI,ND,HIDDEN,NODISC) inline operator span<const T>() const noexcept {
      return span<const T>(m_data, m_size);
    }

    template<typename Other>
    [[nodiscard]] explicit operator span<Other>() const noexcept {
      static_assert(sizeof(Other) == sizeof(T) && alignof(Other) == alignof(T));
      return span<Other>(m_data, m_size);
    }

    J_A(AI,ND,NE,HIDDEN) inline span(u32_t size, T * data) noexcept
      : m_data(data), m_size(size)
    { }
  private:
    T * m_data = nullptr;
    u32_t m_size = 0U;
  };
}
