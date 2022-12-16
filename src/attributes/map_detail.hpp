#pragma once

#include "hzd/type_traits.hpp"
#include "hzd/utility.hpp"
#include "hzd/type_list.hpp"

namespace j::attributes::detail {
  template<typename Arg, typename Tag>
  J_NO_DEBUG constexpr inline bool matches_v = false;

  template<typename Arg>
  J_NO_DEBUG constexpr inline bool matches_v<Arg &, typename Arg::tag_t> = true;

  template<typename Tag, typename... Args>
  consteval u32_t find_first_index() noexcept {
    u32_t i = 0U;
    ((is_same_v<typename Args::tag_t, Tag> || (++i, false)) || ...);
    return i;
  }

  template<typename... Tag>
  struct J_TYPE_HIDDEN without final {};


  template<typename Withouts, typename... Tail>
  struct J_TYPE_HIDDEN get_withouts {
    using type J_NO_DEBUG_TYPE = Withouts;
  };

  template<typename... OldWithouts, typename... NewWithouts, typename... Tail>
  struct J_TYPE_HIDDEN get_withouts<type_list<OldWithouts...>, const without<NewWithouts...> &, Tail...> {
    using type J_NO_DEBUG_TYPE = typename get_withouts<type_list<OldWithouts..., NewWithouts...>, Tail...>::type;
  };

  template<typename... Args>
  using get_withouts_t J_NO_DEBUG_TYPE = typename get_withouts<type_list<>, Args...>::type;
}
