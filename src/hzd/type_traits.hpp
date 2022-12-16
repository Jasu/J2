#pragma once

#include "hzd/utility.hpp"

namespace j {
  // Predicates
  template<typename T>
  J_NO_DEBUG inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(T);

  template<typename... Ts>
  J_NO_DEBUG inline constexpr bool is_trivially_constructible_v = __is_trivially_constructible(Ts...);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_trivially_default_constructible_v = __is_trivially_constructible(T);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_trivially_destructible_v = __is_trivially_destructible(T);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_default_constructible_v = __is_constructible(T);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_nothrow_default_constructible_v = __is_nothrow_constructible(T);

  template<typename T, typename... Args>
  J_NO_DEBUG inline constexpr bool is_nothrow_constructible_v = __is_nothrow_constructible(T, Args...);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_nothrow_copy_constructible_v = __is_nothrow_constructible(T, const T &);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_copy_constructible_v = __is_constructible(T, const T &);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_copy_assignable_v = __is_assignable(T &, const T &);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_move_constructible_v = __is_constructible(T, T &&);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_move_assignable_v = __is_assignable(T &, T &&);

  template<typename T, typename U>
  J_NO_DEBUG inline constexpr bool is_convertible_v = __is_convertible(T, U);

  template<typename T> J_NO_DEBUG inline constexpr bool is_class_v          = __is_class(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_enum_v           = __is_enum(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_signed_v         = __is_signed(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_unsigned_v       = __is_unsigned(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_integral_v       = __is_integral(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_floating_point_v = __is_floating_point(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_const_v          = __is_const(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_scalar_v         = __is_scalar(T);
  template<typename T> J_NO_DEBUG inline constexpr bool is_standard_layout_v         = __is_standard_layout(T);

  template<typename T>
  /// \note `bool` is not included, unlike in normal C++.
  J_NO_DEBUG inline constexpr bool is_arithmetic_v = __is_integral(T) || __is_floating_point(T);
  template<typename T>
  J_NO_DEBUG inline constexpr bool is_signed_integer_v = __is_signed(T) && __is_integral(T);
  template<typename T>
  J_NO_DEBUG inline constexpr bool is_unsigned_integer_v = __is_unsigned(T) && __is_integral(T);



  template<typename T>
  J_NO_DEBUG inline constexpr bool is_member_function_pointer_v = __is_member_function_pointer(T);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_member_object_pointer_v = __is_member_object_pointer(T);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_array_v = __array_rank(T) > 0;

  template<typename T>
  using underlying_type_t J_NO_DEBUG_TYPE = __underlying_type(T);

  template<typename T> struct J_TYPE_HIDDEN remove_ptr           { using type J_NO_DEBUG_TYPE = T; };
  template<typename T> struct J_TYPE_HIDDEN remove_ptr<T*>       { using type J_NO_DEBUG_TYPE = T; };
  template<typename T> struct J_TYPE_HIDDEN remove_ptr<T* const> { using type J_NO_DEBUG_TYPE = T; };

  template<typename T>
  using remove_ptr_t J_NO_DEBUG_TYPE = typename remove_ptr<T>::type;

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_ptr_v = __is_pointer(T);

  template <typename T>
  J_NO_DEBUG static T & declref() noexcept;

  template<typename T>
  J_NO_DEBUG static T && declval_helper(int) noexcept;

  template<typename T>
  J_NO_DEBUG static T declval_helper(long) noexcept;

  template<typename T>
  J_NO_DEBUG static decltype(declval_helper<T>(0)) declval() noexcept;

  template<typename T>
  using decay_t J_NO_DEBUG_TYPE = decltype(identity(declval_helper<T>(0)));

  template<typename T>
  J_NO_DEBUG static bool is_destructible_helper(long);

  template<typename T, typename = decltype(declref<T>().~T())>
  J_NO_DEBUG static void is_destructible_helper(int);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_destructible_v = __is_same(decltype(is_destructible_helper<T>(0)), void);

  template<typename T>
  struct J_TYPE_HIDDEN remove_ref_helper;

  template<typename T>
  struct J_TYPE_HIDDEN remove_ref_helper<T &> {
    using type J_NO_DEBUG_TYPE = T;
  };

  template<typename T> using remove_ref_t J_NO_DEBUG_TYPE = typename remove_ref_helper<T &>::type;

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_ref_v = __is_reference(T);
  template<typename T>
  J_NO_DEBUG inline constexpr bool is_lref_v = __is_lvalue_reference(T);
  template<typename T>
  J_NO_DEBUG inline constexpr bool is_rref_v = __is_rvalue_reference(T);



  template<typename T> struct J_TYPE_HIDDEN remove_ref_or_ptr_helper;
  template<typename T> struct J_TYPE_HIDDEN remove_ref_or_ptr_helper<T * &>         { using type J_NO_DEBUG_TYPE = T; };
  template<typename T> struct J_TYPE_HIDDEN remove_ref_or_ptr_helper<T &>           { using type J_NO_DEBUG_TYPE = T; };

  template<typename T>
  using remove_ref_or_ptr_t J_NO_DEBUG_TYPE = typename remove_ref_or_ptr_helper<T&>::type;



  template<typename T> struct J_TYPE_HIDDEN remove_const;
  template<typename T> struct J_TYPE_HIDDEN remove_const<const T>            { using type J_NO_DEBUG_TYPE = T; };

  template<typename T> using remove_const_t J_NO_DEBUG_TYPE = typename remove_const<const T>::type;



  template<typename T> struct J_TYPE_HIDDEN remove_cref;
  template<typename T> struct J_TYPE_HIDDEN remove_cref<T &>                 { using type = T; };
  template<typename T> struct J_TYPE_HIDDEN remove_cref<const T &>           { using type = T; };

  template<typename T> using remove_cref_t J_NO_DEBUG_TYPE = typename remove_cref<T &>::type;

  template<typename T>         struct J_TYPE_HIDDEN remove_extent            { using type = T; };
  template<typename T>         struct J_TYPE_HIDDEN remove_extent<T[]>       { using type = T; };
  template<typename T, sz_t N> struct J_TYPE_HIDDEN remove_extent<T[N]>      { using type = T; };

  template<typename T> using remove_extent_t J_NO_DEBUG_TYPE = typename remove_extent<T>::type;

  template<bool, typename = void> struct J_TYPE_HIDDEN enable_if;
  template<typename T> struct J_TYPE_HIDDEN enable_if<true, T> { using type J_NO_DEBUG_TYPE = T; };

  template<bool Condition, typename T = void>
  using enable_if_t J_NO_DEBUG_TYPE = typename j::enable_if<Condition, T>::type;

  template<typename T, typename... U> struct J_TYPE_HIDDEN first_type  {using type J_NO_DEBUG_TYPE = T; };
  template<typename T, typename... U>
  using first_type_t J_NO_DEBUG_TYPE = typename first_type<T, U...>::type;

  template<typename Fn, typename... Args>
  J_NO_DEBUG inline constexpr bool is_nothrow_callable_v = noexcept(declval<Fn>()(declval<Args>()...));

  template<typename T>
  concept NicelyCopyable = __is_trivially_copyable(T) || requires {
    typename T::nicely_copyable_tag_t;
  };

  template<typename T>
  concept TaggedNicelyCopyable = !__is_trivially_copyable(T) && requires {
    typename T::nicely_copyable_tag_t;
  };

  template<typename>
  J_NO_DEBUG inline constexpr bool is_nicely_copyable_v = false;

  template<NicelyCopyable T>
  J_NO_DEBUG inline constexpr bool is_nicely_copyable_v<T> = true;

  template<typename T>
  concept ZeroInitializable = __is_scalar(T) || requires {
    typename T::zero_initializable_tag_t;
  };

  template<typename T>
  concept TaggedZeroInitializable = requires {
    typename T::zero_initializable_tag_t;
  };

  template<typename>
  J_NO_DEBUG inline constexpr bool is_zero_initializable_v = false;

  template<ZeroInitializable T>
  J_NO_DEBUG inline constexpr bool is_zero_initializable_v<T> = true;

  template<typename MemPtr> struct J_TYPE_HIDDEN member_traits;

  template<typename C, typename T> struct J_TYPE_HIDDEN member_traits<T C::*> {
    using class_t = J_NO_DEBUG_TYPE C;
    using type_t = J_NO_DEBUG_TYPE T;
  };

  template<typename MemPtr>
  using member_class_t J_NO_DEBUG_TYPE = typename member_traits<MemPtr>::class_t;

  template<typename MemPtr>
  using member_type_t J_NO_DEBUG_TYPE = typename member_traits<MemPtr>::type_t;

  template<typename T, typename U>
  J_NO_DEBUG inline constexpr bool is_same_ignore_cref_v = __is_same(remove_cref_t<T>, remove_cref_t<U>);
}
