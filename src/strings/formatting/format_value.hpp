#pragma once

#include "hzd/concepts.hpp"
#include "type_id/type_id.hpp"
#include "strings/string.hpp"

namespace j::strings::inline formatting {
  enum class inline_type : u8_t {
    ptr,
    ref,

    string,
    uint,
    sint,
    f32,
    f64,
    boolean,
  };

  class format_value final {
  public:
    J_A(AI,ND) inline format_value() noexcept
      : m_value{.p = nullptr}
    { }

    template<SignedInteger T>
    J_A(ND) inline explicit format_value(const T & value) noexcept
      : m_type(typeid(T), (u8_t)inline_type::sint),
        m_value{ .i = value }
    { }

    template<UnsignedInteger T>
    J_A(ND) inline explicit format_value(const T & value) noexcept
      : m_type(typeid(T), (u8_t)inline_type::uint),
        m_value{ .u = value }
    { }

    J_A(ND) inline explicit format_value(const float & value) noexcept
      : m_type(typeid(float), (u8_t)inline_type::f32),
        m_value{ .f = value }
    { }

    J_A(ND) inline explicit format_value(const double & value) noexcept
      : m_type(typeid(double), (u8_t)inline_type::f64),
        m_value{ .d = value }
    { }

    J_A(ND) inline explicit format_value(const bool & value) noexcept
      : m_type(typeid(bool), (u8_t)inline_type::boolean),
        m_value{ .b = value }
    { }

    J_A(ND) inline explicit format_value(const char * J_NOT_NULL value) noexcept
      : m_type(typeid(const char *), (u8_t)inline_type::string),
        m_value{ .sv = value }
    { }

    J_A(ND) inline explicit format_value(const strings::const_string_view & value) noexcept
      : m_type(typeid(strings::const_string_view), (u8_t)inline_type::string),
        m_value{ .sv = value }
    { }

    J_A(ND) inline explicit format_value(const strings::string_view & value) noexcept
      : m_type(typeid(strings::const_string_view), (u8_t)inline_type::string),
        m_value{ .sv = { value.data(), value.size() } }
    { }

    format_value(const strings::string & value) noexcept;

    template<typename T>
    J_A(ND) inline explicit format_value(const T * value) noexcept
      : m_type(typeid(T), (u8_t)inline_type::ptr),
        m_value{ .p = value }
    { }

    J_A(ND) inline format_value(const void * value, const type_id::type_id & type) noexcept
      : m_type(type, (u8_t)inline_type::ptr),
        m_value{ .p = value }
    { }

    template<typename T>
    J_A(ND) inline explicit format_value(const T & value) noexcept
      : m_type(typeid(T), (u8_t)inline_type::ref),
        m_value{ .p = &value }
    { }

    J_INLINE_GETTER inline_type get_inline_type() const noexcept
    { return (inline_type)m_type.tag_bits(); }

    J_INLINE_GETTER const type_id::type_id & type() const noexcept { return m_type; }

    J_INLINE_GETTER u64_t as_uint() const noexcept {
      J_ASSERT((inline_type)m_type.tag_bits() == inline_type::uint);
      return m_value.u;
    }

    J_INLINE_GETTER i64_t as_sint() const noexcept {
      J_ASSERT((inline_type)m_type.tag_bits() == inline_type::sint);
      return m_value.i;
    }

    [[nodiscard]] inline strings::const_string_view as_string_view() const noexcept {
      J_ASSERT((inline_type)m_type.tag_bits() == inline_type::string);
      return m_value.sv;
    }

    [[nodiscard]] inline const void * as_void_star() const noexcept {
      switch ((inline_type)m_type.tag_bits()) {
      case inline_type::ptr:
      case inline_type::ref:
        return m_value.p;
      case inline_type::string:
        return &m_value.sv;
      case inline_type::sint:
        return &m_value.i;
      case inline_type::uint:
        return &m_value.u;
      case inline_type::f32:
        return &m_value.f;
      case inline_type::f64:
        return &m_value.d;
      case inline_type::boolean:
        return &m_value.b;
      }
    }

    template<typename T>
    [[nodiscard]] inline const T * as() const noexcept {
      return reinterpret_cast<const T*>(m_value.p);
    }

  private:
    type_id::type_id m_type;
    union {
      const void * p = nullptr;
      strings::const_string_view sv;
      bool b;
      u64_t u;
      i64_t i;
      float f;
      double d;
    } m_value;
  };
}
