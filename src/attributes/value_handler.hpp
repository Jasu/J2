#pragma once

#include "attributes/common.hpp"
#include "attributes/map_detail.hpp"

namespace j::mem {
  class any;
}

namespace j::attributes::detail {
  template<typename Tag, typename Head>
  J_A(NODISC,ND,AI) constexpr decltype(auto) get_first(Head && head) noexcept {
    static_assert(matches_v<Head &, Tag>, "Named argument not provided.");
    return static_cast<Head &&>(head);
  }

  template<typename Tag, typename Head0, typename Head1, typename... Tail>
  J_A(NODISC,ND,AI) constexpr decltype(auto) get_first(Head0 && head0, Head1 && head1, Tail && ... tail) noexcept {
    if constexpr (matches_v<Head0 &, Tag>) {
      return static_cast<Head0 &&>(head0);
    } else if constexpr (matches_v<Head1 &, Tag>) {
      return static_cast<Head1 &&>(head1);
    } else {
      return get_first<Tag>(static_cast<Tail &&>(tail)...);
    }
  }

  template<typename ValueType, typename Tag, typename Policy>
  class J_TYPE_HIDDEN value_handler;

  /// Logic for handling attributes containing a single value of a specified type.
  template<typename ValueType, typename Tag>
  class J_TYPE_HIDDEN value_handler<ValueType, Tag, single> {
  public:
    J_NO_DEBUG inline constexpr static bool is_multiple_v = false;
    using value_t J_NO_DEBUG_TYPE = ValueType;

    template<typename... Attributes>
    J_INLINE_GETTER_NO_DEBUG constexpr static decltype(auto) get(Attributes && ... attrs) noexcept {
      return get_first<Tag>(static_cast<Attributes &&>(attrs)...).m_value;
    }
  };

  template<typename Fn, typename Tag, typename Keeps, typename... Args>
  struct J_TYPE_HIDDEN apply_helper;

  template<typename Fn, typename Tag, typename... Keeps, typename Head, typename... Tail>
  struct J_TYPE_HIDDEN apply_helper<Fn, Tag, void (Keeps...), Head, Tail...> {
    J_ALWAYS_INLINE_NO_DEBUG constexpr static decltype(auto) apply(Fn && fn, Keeps && ... keeps, Head && head, Tail && ... tail) {
      if constexpr (matches_v<Head &, Tag>) {
        if constexpr ((matches_v<Tail &, Tag> || ...)) {
          return apply_helper<Fn, Tag, void (Keeps..., Head), Tail...>::apply(
            static_cast<Fn &&>(fn),
            static_cast<Keeps &&>(keeps)...,
            static_cast<Head &&>(head),
            static_cast<Tail &&>(tail)...);
        } else {
          return static_cast<Fn &&>(fn)(
            static_cast<Keeps &&>(keeps).m_value...,
            static_cast<Head &&>(head).m_value
          );
        }
      } else {
        return apply_helper<Fn, Tag, void (Keeps...), Tail...>::apply(
          static_cast<Fn &&>(fn),
          static_cast<Keeps &&>(keeps)...,
          static_cast<Tail &&>(tail)...);
      }
    }
  };

  template<typename ValueType, typename Tag>
  class J_TYPE_HIDDEN value_handler<ValueType, Tag, multiple> {
  public:
    using value_t J_NO_DEBUG_TYPE = ValueType;
    J_NO_DEBUG inline constexpr static bool is_multiple_v = true;
  public:
    template<typename... Attributes>
    J_INLINE_GETTER_NO_DEBUG constexpr static decltype(auto) get_first(Attributes && ... attrs) noexcept;

    template<typename Fn, typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG constexpr static decltype(auto) apply(Fn && fn, Args && ... args);

    template<typename Fn, typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG constexpr static decltype(auto) maybe_apply(Fn && fn, Args && ... args);
  };

  template<typename ValueType, typename Tag>
  template<typename... Attributes>
  J_INLINE_GETTER_NO_DEBUG constexpr decltype(auto) value_handler<ValueType, Tag, multiple>::get_first(Attributes && ... attrs) noexcept {
    return detail::get_first<Tag>(static_cast<Attributes &&>(attrs)...).m_value;
  }

  template<typename ValueType, typename Tag>
  template<typename Fn, typename... Args>
  J_ALWAYS_INLINE_NO_DEBUG constexpr decltype(auto) value_handler<ValueType, Tag, multiple>::apply(Fn && fn, Args && ... args) {
    if constexpr ((detail::matches_v<Args &, Tag> || ...)) {
      return apply_helper<Fn, Tag, void (), Args...>::apply(static_cast<Fn &&>(fn), static_cast<Args &&>(args)...);
    } else {
      return static_cast<Fn &&>(fn)();
    }
  }

  template<typename ValueType, typename Tag>
  template<typename Fn, typename... Args>
  J_ALWAYS_INLINE_NO_DEBUG constexpr decltype(auto) value_handler<ValueType, Tag, multiple>::maybe_apply(Fn && fn, Args && ... args) {
    if constexpr ((detail::matches_v<Args &, Tag> || ...)) {
      return apply_helper<Fn, Tag, void (), Args...>::apply(static_cast<Fn &&>(fn), static_cast<Args &&>(args)...);
    }
  }
}
