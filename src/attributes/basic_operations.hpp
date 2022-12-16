#pragma once

#include "attributes/attribute_definition.hpp"

namespace j::attributes {
  template<typename AttributeDef, typename... Args>
  inline constexpr u32_t count_v = (__is_same(typename ::j::remove_ref_t<Args>::tag_t, typename AttributeDef::tag_t) + ... + 0U);

  template<typename AttrDef, typename... Args>
  inline constexpr bool has_v = (__is_same(typename ::j::remove_ref_t<Args>::tag_t, typename AttrDef::tag_t) || ...);

  namespace {
    template<typename Attribute, typename AttrDef>
    consteval bool is(const AttrDef &) noexcept {
      return detail::matches_v<Attribute &, typename AttrDef::tag_t>;
    }

    template<typename AttrDef, typename Attribute>
    consteval bool is(const AttrDef &, const Attribute &) noexcept {
      return detail::matches_v<Attribute &, typename AttrDef::tag_t>;
    }

    template<typename... Args, typename AttrDef>
    consteval bool has(const AttrDef &) noexcept {
      return has_v<AttrDef, Args...>;
    }


    template<typename Arg, typename... Tags>
    [[maybe_unused]] J_HIDDEN J_NO_DEBUG inline constexpr bool matches_any
      = (__is_same(typename ::j::remove_ref_t<Arg>::tag_t, Tags) || ...);

    template<typename... Args, typename... AttributeDefs, u8_t = sizeof...(AttributeDefs) - 2>
    consteval u32_t count(const AttributeDefs & ...) noexcept {
      return (matches_any<Args, typename AttributeDefs::tag_t...> + ... + 0U);
    }

    template<typename... Args, typename AttributeDef>
    consteval u32_t count(const AttributeDef &) noexcept {
      using tag_t = typename AttributeDef::tag_t;
      return (__is_same(typename ::j::remove_ref_t<Args>::tag_t, tag_t) + ... + 0U);
    }
  }
}
