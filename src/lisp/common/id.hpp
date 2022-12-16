#pragma once

#include "lisp/values/imm_type.hpp"
#include "strings/string.hpp"

namespace j::lisp::inline common {
  constexpr inline u8_t package_id_keyword = 0;
  constexpr inline u8_t package_id_global = 1;
  constexpr inline u8_t package_id_packages = 2;
  constexpr inline u8_t package_id_air = 3;
  constexpr inline u8_t package_id_const_fold = 4;
  constexpr inline u8_t package_id_min_user = 5;

  constexpr inline u32_t id_flag_gensym = 1U << 31;
  constexpr inline u32_t id_flag_unresolved = 1U << 30;

  namespace detail {
    enum class unresolved_t : u8_t { v };
    enum class gensym_t : u8_t { v };
  }

  constexpr inline detail::unresolved_t unresolved_v = detail::unresolved_t::v;
  constexpr inline detail::gensym_t gensym_v = detail::gensym_t::v;

  enum raw_t : u8_t { raw_tag };

  struct id_name_ref;

  struct id_name final {
    strings::string pkg;
    strings::string name;

    [[nodiscard]] bool operator==(const id_name &) const noexcept = default;

    inline operator id_name_ref() const noexcept;
  };

  struct id_name_ref final {
    strings::const_string_view pkg;
    strings::const_string_view name;

    [[nodiscard]] bool operator==(const id_name_ref &) const noexcept = default;

    explicit operator id_name() const noexcept {
      return { pkg, name };
    }
  };

  [[nodiscard]] inline bool operator==(const id_name & lhs, const id_name_ref & rhs) noexcept {
    return lhs.pkg == rhs.pkg && lhs.name == rhs.name;
  }

  J_A(AI,NODISC,ND,HIDDEN) inline bool operator==(const id_name_ref & lhs, const id_name & rhs) noexcept {
    return rhs == lhs;
  }

  inline id_name::operator id_name_ref() const noexcept {
    return id_name_ref{ pkg, name };
  }

  struct id final {
    J_BOILERPLATE(id, CTOR_CE, EQ_CE)

    J_A(AI,ND,HIDDEN) explicit constexpr id(null_t) noexcept
      : raw(0U)
    { }

    J_A(AI,ND,HIDDEN) id(raw_t, u32_t raw) noexcept
      : raw(raw)
    { }

    constexpr id(u8_t package_id, u32_t index) noexcept
      : raw(index << 16 | package_id << 8 | (u32_t)tag_sym_id)
    { }

    constexpr id(detail::gensym_t, u8_t package_id, u32_t index) noexcept
      : raw(index << 16 | package_id << 8 | id_flag_gensym | (u32_t)tag_sym_id)
    { }


    constexpr id(detail::unresolved_t, u8_t package_id, u32_t index) noexcept
      : raw(index << 16 | package_id << 8 | id_flag_unresolved | (u32_t)tag_sym_id)
    { }

    J_INLINE_GETTER constexpr bool empty() const noexcept             { return !raw; }
    J_INLINE_GETTER constexpr bool operator!() const noexcept         { return !raw; }
    J_INLINE_GETTER explicit constexpr operator bool() const noexcept { return raw; }

    J_INLINE_GETTER constexpr bool is_unresolved() const noexcept { return id_flag_unresolved & raw; }
    J_INLINE_GETTER constexpr bool is_gensym() const noexcept { return id_flag_gensym & raw; }

    J_INLINE_GETTER constexpr u8_t package_id() const noexcept { return raw >> 8; }
    [[nodiscard]] constexpr u32_t index() const noexcept { return (raw & ~id_flag_unresolved & ~id_flag_gensym) >> 16; }

    [[nodiscard]] constexpr bool is_keyword() const noexcept { return package_id() == package_id_keyword; }
    [[nodiscard]] constexpr bool is_global() const noexcept { return package_id() == package_id_global; }
    [[nodiscard]] constexpr bool is_package_id() const noexcept { return package_id() == package_id_packages; }

    u32_t raw;

    static const id none;
  };
  constexpr inline id id::none{nullptr};
}
