#pragma once

#include "strings/string_view.hpp"
#include "containers/span.hpp"

namespace j::mem {
  struct r_buffer final {
    const char * m_data = nullptr;
    i32_t m_size = 0U;

    J_A(AI,ND,HIDDEN) inline r_buffer() noexcept = default;

    J_A(AI,ND,HIDDEN) inline r_buffer(const void * data, i32_t sz) noexcept
      : m_data((const char *)data),
        m_size(sz)
    { }

    J_A(AI,ND,HIDDEN) inline r_buffer(const void * J_NOT_NULL begin, const void * J_NOT_NULL end) noexcept
      : m_data((const char *)begin),
        m_size((const char *)end - m_data)
    { }

    J_A(ND,NODISC) inline strings::const_string_view get_str(i32_t at, i32_t sz) const noexcept {
      J_ASSERT(at + sz < m_size && m_data);
      return {m_data + at, sz};
    }

    J_A(ND,AI,NODISC,HIDDEN) inline i32_t size() const noexcept {
      return m_size;
    }

    J_A(ND,AI,NODISC,HIDDEN) inline explicit operator bool() const noexcept {
      return m_size;
    }

    J_A(ND,AI,NODISC,HIDDEN) inline bool operator!() const noexcept {
      return !m_size;
    }

    J_A(ND,AI,HIDDEN) inline void reset() noexcept {
      m_data = nullptr;
      m_size = 0U;
    }

    J_A(NODISC,AI,ND,HIDDEN) inline r_buffer slice(i32_t begin, i32_t len) const noexcept {
      J_ASSERT(begin >= 0 && len >= 0 && begin + len <= m_size);
      return {m_data + begin, len};
    }

    J_A(NODISC,AI,ND,HIDDEN) inline r_buffer slice(const void * J_NOT_NULL begin, i32_t len) const noexcept {
      J_ASSERT(begin >= m_data && (const char*)begin - m_data + len <= m_size);
      return {begin, len};
    }

    J_A(NODISC,AI,ND,HIDDEN) inline r_buffer slice(const void * J_NOT_NULL begin, const void * J_NOT_NULL end) const noexcept {
      J_ASSERT(begin >= m_data && end <= m_data + m_size);
      return {begin, end};
    }

    template<typename T>
    J_A(NODISC,AI,ND,HIDDEN,RNN) inline const T* ptr_at(i32_t i) const noexcept {
      J_ASSERT(i >= 0 && i < m_size);
      return (const T *)(m_data + i);
    }

    template<typename T>
    J_A(NODISC,AI,ND,HIDDEN) inline span<const T> to_span() const noexcept {
      return span<const T>(m_data, m_size);
    }

    J_A(NODISC,AI,ND,HIDDEN) inline strings::const_string_view to_str() const noexcept {
      return {m_data, m_size};
    }
  };
}
