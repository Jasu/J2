#pragma once

#include "strings/string_view.hpp"

namespace j::lisp::assembly::disassembly {
  enum class value_type : u8_t {
    none,

    reg,
    address,
    signed_immediate,
    unsigned_immediate,
  };

  enum class value_width : u8_t {
    byte,
    word,
    dword,
    qword,
  };

  namespace detail {
    enum class reg_tag_t                : u8_t { v };
    enum class address_tag_t            : u8_t { v };
    enum class signed_immediate_tag_t   : u8_t { v };
    enum class unsigned_immediate_tag_t : u8_t { v };
  }

  constexpr inline detail::reg_tag_t reg_tag{detail::reg_tag_t::v};
  constexpr inline detail::address_tag_t address_tag{detail::address_tag_t::v};
  constexpr inline detail::signed_immediate_tag_t signed_immediate_tag{detail::signed_immediate_tag_t::v};
  constexpr inline detail::unsigned_immediate_tag_t unsigned_immediate_tag{detail::unsigned_immediate_tag_t::v};

  /// Value - either a named value like register or a label, or a number.
  class value final {
  public:
    /// Construct an empty value.
    J_ALWAYS_INLINE constexpr value() noexcept = default;

    J_ALWAYS_INLINE constexpr value(detail::reg_tag_t, strings::const_string_view reg) noexcept
      : type(value_type::reg),
        name(reg)
    {
      J_ASSERT(reg);
    }

    J_ALWAYS_INLINE value(detail::address_tag_t, void * address) noexcept
      : type(value_type::address),
        m_numeric{ .u = (u64_t)address }
    {
    }

    J_ALWAYS_INLINE value(detail::signed_immediate_tag_t, value_width width, i64_t offset) noexcept
      : type(value_type::signed_immediate),
        width(width),
        m_numeric{ .s = offset }
    {
    }

    J_ALWAYS_INLINE value(detail::unsigned_immediate_tag_t, value_width width, u64_t offset) noexcept
      : type(value_type::unsigned_immediate),
        width(width),
        m_numeric{ .u = offset }
    {
    }

    J_INLINE_GETTER bool empty() const noexcept { return type == value_type::none; }
    J_INLINE_GETTER bool operator!() const noexcept { return type == value_type::none; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return type != value_type::none; }

    J_INLINE_GETTER u64_t as_u64() const noexcept {
      J_ASSUME(type == value_type::unsigned_immediate);
      return m_numeric.u;
    }

    J_INLINE_GETTER u64_t as_raw_u64() const noexcept {
      J_ASSUME(type == value_type::address || type == value_type::signed_immediate || type == value_type::unsigned_immediate);
      return m_numeric.u;
    }

    J_INLINE_GETTER i64_t as_i64() const noexcept {
      J_ASSUME(type == value_type::signed_immediate);
      return m_numeric.s;
    }

    value_type type = value_type::none;
    value_width width = value_width::qword;
    /// Name of the register.
    strings::const_string_view name;
  private:
    union {
      u64_t u;
      i64_t s;
    } m_numeric = { 0U };
  };
}
