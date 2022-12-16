#pragma once

#include "attributes/map_detail.hpp"
#include "hzd/type_traits.hpp"

namespace j::attributes {
  template<typename Tag, typename Fn, typename Attribute>
  J_A(AI,ND,HIDDEN) constexpr void maybe_call(const Fn &, const Attribute &, int = 0) noexcept { }

  template<typename Tag, typename Fn, typename Attribute>
  J_NO_DEBUG inline enable_if_t<detail::matches_v<Attribute &, Tag>> maybe_call(Fn && fn, Attribute && attribute) {
    static_cast<Fn &&>(fn)(static_cast<Attribute &&>(attribute).m_value);
  }

  template<typename Tag, typename Fn, typename Attribute>
  J_A(AI,ND,HIDDEN) constexpr void maybe_call_noexcept(const Fn &, const Attribute &, int = 0) noexcept { }

  template<typename Tag, typename Fn, typename Attribute>
  J_A(ND,HIDDEN) inline enable_if_t<detail::matches_v<Attribute &, Tag>> maybe_call_noexcept(Fn && fn, Attribute && attribute) noexcept {
    static_cast<Fn &&>(fn)(static_cast<Attribute &&>(attribute).m_value);
  }

  template<typename Tag, typename Ptr, typename Attribute>
  J_A(AI,ND,HIDDEN) constexpr void maybe_call_member(const void *, Ptr, const Attribute &) noexcept { }

  template<typename Tag, class Class, class FnClass, typename Result, typename Arg, typename Attribute>
  J_A(ND,HIDDEN) inline enable_if_t<detail::matches_v<Attribute &, Tag>> maybe_call_member(Class * J_NOT_NULL c, Result (FnClass::*fn)(Arg), Attribute && attribute) {
    (c->*fn)(static_cast<Attribute &&>(attribute).m_value);
  }

  template<typename Tag, typename Ptr, typename Attribute>
  J_A(AI,ND,HIDDEN) constexpr void maybe_call_member_noexcept(const void *, Ptr, const Attribute &) noexcept { }

  template<typename Tag, class Class, typename Ptr, typename Attribute>
  J_A(ND,HIDDEN) inline enable_if_t<detail::matches_v<Attribute &, Tag>> maybe_call_member_noexcept(Class * c, Ptr fn, Attribute && attribute) noexcept {
    (c->*fn)(static_cast<Attribute &&>(attribute).m_value);
  }

  template<typename AttrDef, typename Fn, typename... Attributes>
  J_A(ND,HIDDEN) inline void foreach(const AttrDef &, Fn && fn, Attributes && ... attributes) {
    (maybe_call<typename AttrDef::tag_t>(static_cast<Fn &&>(fn), static_cast<Attributes &&>(attributes)), ...);
  }

  template<typename AttrDef, typename Fn, typename... Attributes>
  J_A(ND,HIDDEN) inline void foreach_noexcept(const AttrDef &, Fn && fn, Attributes && ... attributes) noexcept {
    (maybe_call_noexcept<typename AttrDef::tag_t>(static_cast<Fn &&>(fn), static_cast<Attributes &&>(attributes)), ...);
  }

  template<typename AttrDef, class Class, typename FnClass, typename Result, typename Arg, typename... Attributes>
  J_A(ND,HIDDEN) inline void foreach_member(const AttrDef &, Class & c, Result (FnClass::*fn)(Arg), Attributes && ... attributes) {
    (maybe_call_member<typename AttrDef::tag_t>(&c, fn, static_cast<Attributes &&>(attributes)), ...);
  }

  template<typename AttrDef, class Class, typename FnClass, typename Result, typename Arg, typename... Attributes>
  J_A(ND,HIDDEN) inline void foreach_member_noexcept(const AttrDef &,
                                                Class & c,
                                                Result (FnClass::*fn)(Arg),
                                                Attributes && ... attributes) noexcept
  {
    (maybe_call_member_noexcept<typename AttrDef::tag_t>(&c, fn, static_cast<Attributes &&>(attributes)), ...);
  }
}
