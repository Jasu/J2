#pragma once

#include "strings/string.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering {
  class attribute_key final {
  public:
    J_A(AI,ND) constexpr attribute_key() noexcept = default;

    inline explicit attribute_key(strings::const_string_view str)
      : m_value(str)
    { }

    inline explicit attribute_key(strings::string && str) noexcept
      : m_value(static_cast<strings::string &&>(str))
    { }

    inline explicit attribute_key(const char * const str)
      : m_value(str)
    { }

    inline explicit attribute_key(i32_t index) noexcept
      : m_value(strings::not_a_string_tag, index)
    { }

    J_A(ND,AI,NODISC) bool is_index() const noexcept
    { return m_value.is_not_a_string(); }

    J_A(ND,AI,NODISC) bool is_name() const noexcept
    { return (bool)m_value; }

    J_A(ND,AI,NODISC) const strings::string & name() const noexcept {
      return m_value;
    }

    J_A(ND,AI,NODISC) i32_t index() const noexcept {
      return m_value.not_a_string_value();
    }

    J_A(ND,AI,NODISC) explicit operator bool() const noexcept
    { return (bool)m_value; }

    J_A(ND,AI,NODISC) bool operator!() const noexcept
    { return !m_value; }

    J_A(ND,AI,NODISC)  bool empty() const noexcept
    { return !m_value; }

  private:
    strings::string m_value;
  };
}
