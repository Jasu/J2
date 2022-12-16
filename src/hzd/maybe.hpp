#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/type_traits.hpp"

namespace j {
  namespace detail {
    enum class no_sentinel_tag { v };
  }

  /// An optional value.
  template<typename T, auto Sentinel = detail::no_sentinel_tag::v>
  struct maybe final {
    J_A(AI,ND) inline maybe() noexcept = default;

    J_A(AI,ND) inline explicit maybe(T && val) noexcept
      : m_value(static_cast<T &&>(val))
    { }

    inline explicit maybe(const T & val) noexcept(is_nothrow_copy_constructible_v<T>)
      : m_value(val)
    { }

    J_A(AI,NODISC,ND) inline explicit operator bool() const noexcept
    { return m_value != Sentinel; }

    J_A(AI,NODISC,ND) inline bool operator!() const noexcept
    { return m_value == Sentinel; }

    J_A(AI,NODISC,ND) inline bool empty() const noexcept
    { return m_value == Sentinel; }

    J_A(AI,NODISC,ND) inline bool is_valid() const noexcept
    { return m_value != Sentinel; }

    [[nodiscard]] inline T & value() noexcept {
      J_ASSERT(m_value != Sentinel);
      return m_value;
    }

    [[nodiscard]] inline const T & value() const noexcept {
      J_ASSERT(m_value != Sentinel);
      return m_value;
    }

    [[nodiscard]] inline T & value_unsafe() noexcept {
      return m_value;
    }

    [[nodiscard]] inline const T & value_unsafe() const noexcept {
      return m_value;
    }


    [[nodiscard]] inline explicit operator const T &() const noexcept {
      J_ASSERT(m_value != Sentinel);
      return m_value;
    }
    [[nodiscard]] inline explicit operator T &() noexcept {
      J_ASSERT(m_value != Sentinel);
      return m_value;
    }
  private:
    T m_value = Sentinel;
  };

  template<typename T, detail::no_sentinel_tag Tag>
  struct maybe<T, Tag> final {
    J_A(AI,ND) inline maybe() noexcept = default;

    J_A(AI,ND) inline explicit maybe(T && val) noexcept
      : m_value(static_cast<T &&>(val)),
        m_is_valid(true)
    { }

    inline explicit maybe(const T & val) noexcept(is_nothrow_copy_constructible_v<T>)
      : m_value(static_cast<T &&>(val)),
        m_is_valid(true)
    { }

    J_A(AI,ND,NODISC,HIDDEN) inline explicit operator bool() const noexcept
    { return m_is_valid; }

    J_A(AI,ND,NODISC,HIDDEN) inline bool operator!() const noexcept
    { return !m_is_valid; }

    J_A(AI,ND,NODISC,HIDDEN) inline bool empty() const noexcept
    { return !m_is_valid; }

    J_A(AI,ND,NODISC,HIDDEN) inline bool is_valid() const noexcept
    { return m_is_valid; }

    [[nodiscard]] inline T & value() noexcept {
      J_ASSERT(m_is_valid);
      return m_value;
    }

    [[nodiscard]] inline const T & value() const noexcept {
      J_ASSERT(m_is_valid);
      return m_value;
    }

    [[nodiscard]] inline T & value_unsafe() noexcept {
      return m_value;
    }

    [[nodiscard]] inline const T & value_unsafe() const noexcept {
      return m_value;
    }

    [[nodiscard]] inline explicit operator T &() noexcept {
      J_ASSERT(m_is_valid);
      return m_value;
    }

    [[nodiscard]] inline explicit operator const T &() const noexcept {
      J_ASSERT(m_is_valid);
      return m_value;
    }
  private:
    T m_value;
    bool m_is_valid = false;
  };
}
