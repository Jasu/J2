#pragma once

#include "hzd/type_traits.hpp"
#include "attributes/common.hpp"

namespace j::attributes {
  namespace detail {
    template<typename...> struct without;

    template<typename Attr>
    J_NO_DEBUG J_HIDDEN inline constexpr bool enable_if_attributes_helper_v = false;

    template<typename Def, typename Arg>
    J_NO_DEBUG J_HIDDEN inline constexpr bool enable_if_attributes_helper_v<attribute<Def, Arg>> = true;

    template<typename Tag>
    J_NO_DEBUG J_HIDDEN inline constexpr bool enable_if_attributes_helper_v<attribute_definition<bool, Tag, flag, void>> = true;

    template<typename... Tags>
    J_NO_DEBUG J_HIDDEN inline constexpr bool enable_if_attributes_helper_v<without<Tags...>> = true;
  }

  template<typename... Args>
  using enable_if_attributes_t J_NO_DEBUG_TYPE = enable_if_t<
    detail::enable_if_attributes_helper_v<remove_cref_t<__type_pack_element<0, Args...>>>>;
}
