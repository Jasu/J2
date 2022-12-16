#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/string.hpp"
#include "strings/common.hpp"

namespace j::strings {
  template<typename> class basic_string_view;

  using string_view = basic_string_view<char>;
  using const_string_view = basic_string_view<const char>;

  template<typename Char>
  class J_AT(PREF_NAME(string_view),PREF_NAME(const_string_view)) basic_string_view {
  public:
    using nicely_copyable_tag_t J_NO_DEBUG_TYPE = void;
    using zero_initializable_tag_t J_NO_DEBUG_TYPE = void;

    J_AV(HIDDEN,ND) inline static constexpr i32_t npos = -1;

    /// Construct an empty string view.
    J_A(AI,ND,HIDDEN) inline constexpr basic_string_view() noexcept = default;

    /// Construct a string view given data and size.
    J_A(AI,ND,HIDDEN) inline constexpr basic_string_view(Char * J_AA(NN) const data, i32_t size) noexcept
      : m_data(data),
        m_size(size)
    { }

    /// Construct a string view given begin and end pointers.
    J_A(AI,ND,HIDDEN) inline constexpr basic_string_view(Char * J_AA(NN) const begin,
                                               Char * J_AA(NN) const end) noexcept
      : m_data(begin),
        m_size(end - begin)
    { }

    /// Construct a string view from a null-terminated string, excluding the null terminator.
    inline constexpr basic_string_view(Char * J_AA(NN) c_string) noexcept
      : m_data(c_string),
        m_size(::j::strlen(c_string))
    { }

    J_A(AI,NODISC) inline constexpr i32_t size() const noexcept   { return m_size; }
    J_A(AI,NODISC) inline constexpr Char * data() const noexcept  { return m_data; }

    J_A(AI,NODISC) inline constexpr Char * begin() const noexcept { return m_data; }
    J_A(AI,NODISC) inline constexpr Char * end() const noexcept   { return m_data + m_size; }

    J_A(AI,NODISC) inline constexpr Char & front() const noexcept { return *m_data; }

    J_A(AI,NODISC) inline constexpr Char & back() const noexcept { return m_data[m_size - 1]; }

    J_A(AI,NODISC) constexpr Char & operator[](i32_t index) const noexcept { return m_data[index]; }

    [[nodiscard]] constexpr Char & at(i32_t index) const {
      J_REQUIRE(index >= 0 && index < m_size, "Out of range.");
      return m_data[index];
    }

    /// Remove a prefix from the string view, returning it.
    [[nodiscard]] inline basic_string_view take_prefix(i32_t len) noexcept {
      if (len < 0) { len = m_size; }
      basic_string_view result(m_data, len);
      return m_data += len, m_size -= len, result;
    }

    /// Remove a suffix from the string view, returning it.
    [[nodiscard]] inline basic_string_view take_suffix(i32_t len) noexcept {
      if (len < 0) { len = m_size; }
      return m_size -= len, basic_string_view(m_data + m_size, len);
    }

    /// Remove a prefix from the string view, returning this.
    inline basic_string_view & remove_prefix(i32_t len) noexcept {
      if (len < 0) { len = m_size; }
      m_data += len, m_size -= len;
      return *this;
    }

    /// Remove a prefix from the string view, returning this.
    inline basic_string_view & remove_suffix(i32_t len) noexcept {
      m_size -= (len < 0 ? m_size : len);
      return *this;
    }

    /// Get a prefix of the string view.
    [[nodiscard]] inline basic_string_view prefix(i32_t len) const noexcept {
      return basic_string_view(m_data, len < 0 ? m_size : len);
    }

    /// Get a suffix of the string view.
    [[nodiscard]] inline basic_string_view suffix(i32_t len) const noexcept {
      if (len < 0) { len = m_size; }
      return basic_string_view{m_data + m_size - len, len};
    }

    /// Get a prefix of the string view by removing [*len]() characters.
    [[nodiscard]] inline basic_string_view without_suffix(i32_t len) const noexcept {
      return basic_string_view(m_data, len > 0 ? m_size - len : 0);
    }

    /// Get a suffix of the string view by removing [*len]() characters.
    [[nodiscard]] inline basic_string_view without_prefix(i32_t len) const noexcept {
      if (len < 0) { len = m_size; }
      return basic_string_view(m_data + len, m_size - len);
    }

    /// Get a substring of the string view by start position and length.
    [[nodiscard]] inline basic_string_view slice(i32_t start, i32_t len) const noexcept {
      return basic_string_view(m_data + start, len < 0 ? m_size - start : len);
    }

    /// Set the string view to an empty string.
    J_A(AI,ND,HIDDEN) inline void clear() noexcept { m_size = 0; }

    /// Return whether the string view is not empty.
    J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept { return m_size; }
    /// Return whether the string view is empty.
    J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept { return !m_size; }
    /// Return whether the string view is empty.
    J_A(AI,NODISC,ND,HIDDEN) inline bool empty() const noexcept { return !m_size; }

    /// Conversion to const version of the string view.
    J_A(AI,NODISC,ND) inline operator basic_string_view<const char>() const noexcept
    { return basic_string_view<const char>(m_data, m_size); }


    J_A(AI,NODISC,ND,HIDDEN) bool is_null_terminated() const noexcept {
      return m_data && & m_data[m_size];
    }

    Char * m_data = nullptr;
    i32_t m_size = 0;
  };

  inline constexpr bool operator==(const_string_view lhs, const_string_view rhs) noexcept {
    return lhs.m_size == rhs.m_size && ::j::memcmp(lhs.m_data, rhs.m_data, lhs.m_size) == 0;
  }

  [[nodiscard]] inline constexpr bool operator==(const char * J_NOT_NULL lhs, const_string_view rhs) noexcept {
    return !j::strncmp(lhs, rhs.m_data, rhs.m_size) && !lhs[rhs.m_size];
  }

  J_A(AI,NODISC,ND,HIDDEN) inline constexpr bool operator==(const_string_view lhs, const char * J_NOT_NULL rhs) noexcept {
    return operator==(rhs, lhs);
  }

  bool operator<(const_string_view lhs, const_string_view rhs) noexcept;
}
