#pragma once

#include "hzd/utility.hpp"

namespace j {
  /// For passing parameter packs.
  ///
  /// \note Declaration-only.
  template<typename... Ts> struct type_list;

  namespace detail {
    template<typename T>
    J_NO_DEBUG consteval u32_t invalid_template_argument() {
      static_assert(__is_same(T, T*), "Template argument is not type list.");
      return 0;
    }

    template<u32_t I, typename T>
    struct J_TYPE_HIDDEN type_list_at;

    template<u32_t I, typename... Ts>
    struct J_TYPE_HIDDEN type_list_at<I, type_list<Ts...>> final {
      using type J_NO_DEBUG_TYPE = __type_pack_element<I, Ts...>;
    };

    template<template <typename> typename Fn, typename Accept, typename... Ts>
    struct J_TYPE_HIDDEN type_list_filter;

    template<template <typename> typename Fn, typename Accept>
    struct J_TYPE_HIDDEN type_list_filter<Fn, Accept> final {
      using type J_NO_DEBUG_TYPE = Accept;
    };

    template<typename List, typename Item>
    struct J_TYPE_HIDDEN type_list_contains;

    template<typename... Items, typename Item>
    struct J_TYPE_HIDDEN type_list_contains<type_list<Items...>, Item> final {
      J_NO_DEBUG constexpr inline static bool value = (is_same_v<Items, Item> | ...);
    };

    template<template <typename> typename Fn, typename... Accepts, typename Head, typename... Tail>
    struct J_TYPE_HIDDEN type_list_filter<Fn, type_list<Accepts...>, type_list<Head, Tail...>> final {
      using type J_NO_DEBUG_TYPE = typename type_list_filter<
        Fn,
        conditional_t<
          Fn<Head>::value,
          type_list<Accepts..., Head>,
          type_list<Accepts...>>,
        Tail...
      >::type;
    };

    template<template <typename> typename Fn, typename... Ts>
    struct J_TYPE_HIDDEN type_list_remove;

    template<template <typename> typename Fn, typename Accept>
    struct J_TYPE_HIDDEN type_list_remove<Fn, Accept> final {
      using type J_NO_DEBUG_TYPE = Accept;
    };

    template<template <typename> typename Fn, typename... Accepts, typename Head, typename... Tail>
    struct J_TYPE_HIDDEN type_list_remove<Fn, type_list<Accepts...>, Head, Tail...> final {
      using type J_NO_DEBUG_TYPE = typename type_list_remove<
        Fn,
        conditional_t<
          !Fn<Head>::value,
          type_list<Accepts..., Head>,
          type_list<Accepts...>>,
        Tail...
      >::type;
    };

    template<template <typename> typename Fn, typename List>
    struct J_TYPE_HIDDEN type_list_remove_helper;

    template<template <typename> typename Fn, typename... Ts>
    struct J_TYPE_HIDDEN type_list_remove_helper<Fn, type_list<Ts...>> final {
      using type J_NO_DEBUG_TYPE = typename type_list_remove<Fn, type_list<>, Ts...>::type;
    };

    template<typename List, typename... Accept>
    struct J_TYPE_HIDDEN type_list_remove_voids;

    template<typename... Accept>
    struct J_TYPE_HIDDEN type_list_remove_voids<type_list<>, Accept...> final {
      using type J_NO_DEBUG_TYPE = type_list<Accept...>;
    };

    template<typename Head, typename... Tail, typename... Accept>
    struct J_TYPE_HIDDEN type_list_remove_voids<type_list<Head, Tail...>, Accept...> final {
      using type J_NO_DEBUG_TYPE = typename type_list_remove_voids<type_list<Tail...>, Accept..., Head>::type;
    };

    template<typename... Tail, typename... Accept>
    struct J_TYPE_HIDDEN type_list_remove_voids<type_list<void, Tail...>, Accept...> final {
      using type J_NO_DEBUG_TYPE = typename type_list_remove_voids<type_list<Tail...>, Accept...>::type;
    };

    template<template <typename> typename Fn, typename T>
    struct J_TYPE_HIDDEN type_list_map;

    template<template <typename> typename Fn, typename... Ts>
    struct J_TYPE_HIDDEN type_list_map<Fn, type_list<Ts...>> {
      using type J_NO_DEBUG_TYPE = type_list<typename Fn<Ts>::type...>;
    };

    template<typename... Lists>
    struct J_TYPE_HIDDEN type_list_concat;

    template<typename List>
    struct J_TYPE_HIDDEN type_list_concat<List> {
      using type J_NO_DEBUG_TYPE = List;
    };

    template<typename... Ts, typename... Us>
    struct J_TYPE_HIDDEN type_list_concat<type_list<Ts...>, type_list<Us...>> {
      using type J_NO_DEBUG_TYPE = type_list<Ts..., Us...>;
    };

    template<typename A, typename B, typename... Rest>
    struct J_TYPE_HIDDEN type_list_concat<A, B, Rest...> {
      using type J_NO_DEBUG_TYPE = typename type_list_concat<typename type_list_concat<A, B>::type, Rest...>::type;
    };

  }



  /// Evaluates to the size of `type_list` `T`.
  ///
  /// If the argument `T` is not a `type_list`, fails with a static assert.
  template<typename T>
  J_NO_DEBUG constexpr inline u32_t type_list_size = detail::invalid_template_argument<T>();
  template<typename... Ts>
  J_NO_DEBUG constexpr inline u32_t type_list_size<type_list<Ts...>> = sizeof...(Ts);



  /// Gets the `I`th element of type list `T`.
  template<u32_t I, typename T>
  using type_list_at_t J_NO_DEBUG_TYPE = detail::type_list_at<I, T>;


  /// Replace each element in `type_list` `T` with `Fn<Element>::type`.
  template<typename List, template <typename> typename Fn>
  using type_list_map_t J_NO_DEBUG_TYPE = typename detail::type_list_map<Fn, List>::type;

  template<typename List, template <typename> typename Fn>
  using type_list_filter_t J_NO_DEBUG_TYPE = typename detail::type_list_filter<Fn, type_list<>, List>::type;

  template<typename List, template <typename> typename Fn>
  using type_list_remove_t J_NO_DEBUG_TYPE = typename detail::type_list_remove_helper<Fn, List>::type;

  template<typename List>
  using type_list_remove_voids_t J_NO_DEBUG_TYPE = typename detail::type_list_remove_voids<List>::type;


  template<typename... Lists>
  using type_list_concat_t J_NO_DEBUG_TYPE = detail::type_list_concat<Lists...>;

  template<typename List, typename Item>
  J_NO_DEBUG inline constexpr bool type_list_contains_v = detail::type_list_contains<List, Item>::value;
}
