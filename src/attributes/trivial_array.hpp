#pragma once

#include "containers/trivial_array_fwd.hpp"
#include "attributes/basic_operations.hpp"
#include "attributes/foreach.hpp"

namespace j::attributes {
  template<typename T, typename AttrDef, typename... Attrs>
  J_A(AI,ND,HIDDEN,NODISC) inline trivial_array<T> as_trivial_array_move(const AttrDef &, Attrs && ... attrs) {
    constexpr u32_t cnt = count_v<AttrDef, Attrs...>;
    if constexpr (cnt > 0U) {
      trivial_array<T> result(containers::uninitialized, cnt);
      (maybe_call_member_noexcept<typename AttrDef::tag_t>(
        &result,
        static_cast<T& (trivial_array<T>::*) (T &&) noexcept>(
          &trivial_array<T>::initialize_element),
        static_cast<Attrs &&>(attrs)), ...);
      return result;
    } else {
      return trivial_array<T>();
    }
  }

  template<typename T, typename AttrDef, typename... Attrs>
  J_A(AI,ND,HIDDEN,NODISC) inline trivial_array_copyable<T> as_trivial_array_copyable_move(const AttrDef &, Attrs && ... attrs) {
    constexpr u32_t cnt = count_v<AttrDef, Attrs...>;
    if constexpr (cnt > 0U) {
      trivial_array_copyable<T> result(containers::uninitialized, cnt);
      (maybe_call_member_noexcept<typename AttrDef::tag_t>(
        &result,
        static_cast<T & (trivial_array_copyable<T>::*) (T &&) noexcept>(
          &trivial_array_copyable<T>::initialize_element),
        static_cast<Attrs &&>(attrs)), ...);
      return result;
    } else {
      return trivial_array_copyable<T>();
    }
  }

  template<typename T, typename AttrDef, typename... Attrs>
  J_A(AI,ND,HIDDEN) inline void populate_trivial_array_move(trivial_array<T> & target,
                                                     const AttrDef &, Attrs && ... attrs) noexcept
  {
    (maybe_call_member_noexcept<typename AttrDef::tag_t>(
      &target,
      static_cast<T & (trivial_array<T>::*) (T &&) noexcept>(&trivial_array<T>::initialize_element),
      static_cast<Attrs &&>(attrs)), ...);
  }

  template<typename T, typename AttrDef, typename... Attrs>
  J_A(AI,ND,HIDDEN) inline void populate_trivial_array_move(trivial_array_copyable<T> & target,
                                                     const AttrDef & def, Attrs && ... attrs) noexcept
  {
    foreach_member_noexcept(def,
                            target,
                            static_cast<T & (trivial_array_copyable<T>::*) (T &&) noexcept>(
                              &trivial_array_copyable<T>::initialize_element),
                            static_cast<Attrs &&>(attrs)...);
  }
}
