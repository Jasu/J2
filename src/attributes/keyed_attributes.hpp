#pragma once

#include "attributes/common.hpp"
#include "attributes/attribute.hpp"
#include "hzd/type_traits.hpp"

namespace j::attributes {
  template<typename KeyType, typename ValueType>
  struct attribute_pair final {
    KeyType key;
    ValueType value;
  };
}

namespace j::attributes::detail {
  template<typename KeyType, typename AttrDef>
  struct assign_wrapper_any {
    KeyType m_key;

    template<typename Arg>
    constexpr auto operator=(Arg && rhs) noexcept(noexcept(consume(declval<Arg>()))) {
      return attribute<AttrDef, attribute_pair<KeyType, decay_t<Arg>>>{{
          static_cast<KeyType &&>(m_key),
          static_cast<Arg &&>(rhs)
        }};
    }

    assign_wrapper_any & operator=(const assign_wrapper_any &) = delete;
  };


  template<typename KeyType, typename ValueType, typename AttrDef>
  struct assign_wrapper {
    KeyType m_key;

    template<typename Arg>
    constexpr attribute<AttrDef, ValueType> operator=(Arg && rhs) noexcept(is_nothrow_constructible_v<ValueType, KeyType &&, Arg &&>) {
      return {ValueType{static_cast<KeyType &&>(m_key), static_cast<Arg &&>(rhs)}};
    }

    assign_wrapper & operator=(const assign_wrapper &) = delete;
  };
}
