#pragma once

#include "hzd/type_traits.hpp"

namespace j {
  /// Concept satisfied by trivially copyable types.
  template<typename Base, typename Derived>
  concept DerivedFrom = __is_base_of(Base, Derived);

  /// Concept satisfied by trivially copyable types.
  template<typename Base, typename Derived>
  concept BaseOf = __is_base_of(Derived, Base);

  /// Concept satisfied by trivially copyable types.
  template<typename From, typename To>
  concept ConvertibleTo = __is_convertible(From, To);

  template<typename To, typename From>
  concept ConvertibleFrom = __is_convertible(From, To);

  /// Concept satisfied by trivially copyable types.
  template<typename T>
  concept TriviallyCopyable = __is_trivially_copyable(T);

  /// Concept satisfied by copy-constructible types.
  template<typename T>
  concept CopyConstructible = __is_constructible(T, const T &);

  template<typename T>
  concept Scalar = __is_scalar(T);

  template<typename T>
  concept Scalarish = __is_scalar(T) || (sizeof(T) <= 8 && __is_trivially_destructible(T));

  /// Concept satisfied by trivially destructible types.
  template<typename T>
  concept TriviallyDestructible = __is_trivially_destructible(T);

  /// Concept satisfied by nothrow default constructible types.
  template<typename T>
  concept NothrowDefaultConstructible = ::j::is_nothrow_default_constructible_v<T>;

  /// Concept satisfied by nothrow default constructible types.
  template<typename T, typename... Args>
  concept NothrowConstructible = ::j::is_nothrow_constructible_v<T, Args...>;

  /// Concept satisfied by integral types, including bool.
  ///
  /// Pointers and enums are not integral types.
  template<typename T>
  concept Integral = __is_integral(T);

  /// Concept satisfied by integral types, excluding bool.
  ///
  /// Pointers and enums are not integral types.
  template<typename T>
  concept ActuallyIntegral = __is_integral(T) && !is_same_v<T, bool>;

  template<typename T>
  concept NotIntegral = !__is_integral(T);

  /// Concept satisfied by integers and floating point numbers, and *NOT* by `bool`.
  ///
  /// \note `bool` is not included, unlike in normal C++.
  template<typename T>
  concept Arithmetic = __is_integral(T) || __is_floating_point(T);

  template<typename T>
  concept UnsignedInteger = __is_integral(T) && !__is_signed(T);

  template<typename T>
  concept SignedInteger = __is_integral(T) && __is_signed(T);

  template<typename T>
  concept FloatingPoint = __is_floating_point(T);

  template<typename T>
  concept Pointer = __is_pointer(T);

  template<typename T>
  concept Signed = __is_signed(T);

  template<typename T, typename U>
  concept SameAs = __is_same(T, U);

  template<typename T, typename... U>
  concept OneOf = (__is_same(T, U) || ...);

  template<typename T, typename... U>
  concept OtherThan = !(__is_same(T, U) || ...);

  template<typename T, typename Fn>
  concept UnaryPredicate = requires(const T t, Fn pred) {
    { pred(t) } -> SameAs<bool>;
  };

  template<typename T, typename Fn>
  concept BinaryPredicate = requires(const T t, Fn pred) {
    { pred(t, t) } -> SameAs<bool>;
  };

  template<typename T>
  concept Swappable = requires(T a) {
    { a.swap(a) } noexcept;
  };

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_swappable_v = false;

  template<Swappable T>
  J_NO_DEBUG inline constexpr bool is_swappable_v<T> = true;

  template<typename T>
  concept Subtractable = requires(const T t) {
    { t - t };
  };

  template<typename T, typename U>
  concept Iterable = requires(const U t) {
    { *t.begin() } -> SameAs<const T &>;
    { *t.end() } -> SameAs<const T &>;
  };

  template<typename U, typename T>
  concept SamePtrConst = __is_same(T *, U) || __is_same(const T *, U);

  template<typename U, typename T>
  concept PtrRegion = requires(U & t) {
    { t.begin() } -> ConvertibleTo<T *>;
    { t.size() };
  };

  template<typename U, typename T>
  concept ConstPtrRegion = requires(const U t) {
    { t.begin() } -> ConvertibleTo<T *>;
    { t.size() };
  };

  template<typename T>
  concept NonArray = !__is_array(remove_cref_t<T>);
  template<typename T>
  concept Array = __is_array(remove_cref_t<T>);

  template<typename T>
  concept NonRef = !__is_reference(T);

  template<typename T>
  concept NonRefNonArray = !__is_reference(T) && __array_rank(T) == 0;

  template<typename T>
  concept NonRefNonArrayNonScalar = !__is_reference(T) && __array_rank(T) == 0 && !__is_scalar(T);

  template<typename T>
  concept NonRefNonArrayNotTriviallyCopyable = !__is_reference(T) && __array_rank(T) == 0 && !__is_trivially_copyable(T);

  template<typename T>
  concept NonArrayNonScalar = !__is_array(T) && !__is_scalar(T);

  template<typename T>
  concept NonArrayPlain = !__is_array(T);
}
