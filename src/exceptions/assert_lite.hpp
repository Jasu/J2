#pragma once

#include "hzd/type_traits.hpp"

namespace j::strings {
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
}

namespace j::exceptions {
  namespace detail {
    [[noreturn]] void throw_range_signed(
      const char * msg, i64_t left, i64_t value, i64_t right, i32_t skip_frames = 0);

    [[noreturn]] void throw_range_signed_0(
      const char * msg, i64_t value, i64_t right, i32_t skip_frames = 0);

    [[noreturn]] void fail_range_signed(
      const char * msg, i64_t left, i64_t value, i64_t right, i32_t skip_frames = 0) noexcept;

    [[noreturn]] void throw_range_unsigned(
      const char * msg, u64_t left, u64_t value, u64_t right, i32_t skip_frames = 0);

    [[noreturn]] void fail_range_unsigned(
      const char * msg, u64_t left, u64_t value, u64_t right, i32_t skip_frames = 0) noexcept;

    [[noreturn]] void throw_range_pointer(
      const char * msg, const void * left, const void * value, const void * right, i32_t skip_frames = 0);

    [[noreturn]] void fail_range_pointer(
      const char * msg, const void * left, const void * value, const void * right, i32_t skip_frames = 0) noexcept;

  }
  /// Throws [j::exceptions::out_of_range_exception].
  J_A(NORET) void throw_out_of_range(i32_t skip_frames);

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range(i32_t skip_frames, u32_t index);
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range(i32_t skip_frames, u32_t index, u32_t max);
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0(i32_t index);
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0(i32_t index, i32_t max);

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0_skip(i32_t skip_frames, i32_t index);
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0_skip(i32_t skip_frames, i32_t index, i32_t max);

  /// Throws [j::exceptions::out_of_range_exception]
  J_A(NORET) void fail_out_of_range() noexcept;
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void fail_out_of_range(i32_t skip_frames, u32_t index) noexcept;
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void fail_out_of_range(i32_t skip_frames, u32_t index, u32_t max) noexcept;
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void fail_out_of_range_0(i32_t index) noexcept;
  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void fail_out_of_range_0(i32_t index, i32_t max) noexcept;

  /// Throws [j::exceptions::logic_error_exception] with the index specified.
  J_A(NORET) void throw_out_of_range(i32_t skip_frames, strings::const_string_view index);

  /// Throws [j::exceptions::system_error_exception] and resets errno.
  J_A(NORET) void throw_system_error_exception(i32_t skip_frames, const char * msg);

  J_A(NORET) void throw_system_error_exception(i32_t skip_frames, const char * msg, i32_t fd);

  /// Throws [j::exceptions::logic_error_exception] with the message specified.
  J_A(NORET) void throw_logic_error_exception(const char * msg, i32_t skip_frames = 0);

  /// Throws [j::exceptions::logic_error_exception] with the message specified.
  J_A(NORET) void fail_logic_error_exception(const char * msg, i32_t skip_frames = 0) noexcept;

  /// Throws [j::exceptions::exception] with the message specified.
  J_A(NORET) void throw_exception(const char * msg, i32_t skip_frames = 0);

  /// Throws [j::exceptions::exception] with the message specified.
  J_A(NORET) void fail_exception(const char * msg, i32_t skip_frames = 0) noexcept;

  /// Throws [j::exceptions::exception] with the message specified.
  J_A(NORET) void fail_todo(const char * todo, i32_t skip_frames = 0) noexcept;

  /// Throws [j::exceptions::logic_error_exception] that [param] cannot be null.
  J_A(NORET) void throw_not_null(const char * J_NOT_NULL param, i32_t skip_frames = 0);

  /// Throws [j::exceptions::logic_error_exception] that [param] cannot be null.
  J_A(NORET) void fail_not_null(const char * J_NOT_NULL param, const char * J_NOT_NULL file, i32_t skip_frames = 0) noexcept;

  /// Throws [j::exceptions::logic_error_exception] that [param] cannot be null.
  template<typename V>
  J_A(ND,AI,HIDDEN,NORET) void throw_range(const char * msg, type_identity_t<V> left,
                                           V value,
                                           type_identity_t<V> right, i32_t skip_frames = 0)
  {
    if constexpr (is_ptr_v<V>) {
      detail::throw_range_pointer(msg, left, value, right, skip_frames + 1);
    } else if constexpr (is_signed_v<V>) {
      detail::throw_range_signed(msg, left, value, right, skip_frames + 1);
    } else {
      static_assert(is_integral_v<V>);
      detail::throw_range_unsigned(msg, left, value, right, skip_frames + 1);
    }
  }

  /// Throws [j::exceptions::logic_error_exception] that [param] cannot be null.
  template<typename V>
  J_A(ND,AI,HIDDEN,NORET) inline void fail_range(const char * msg,
                                                 type_identity_t<V> left,
                                                 V value,
                                                 type_identity_t<V> right,
                                                 i32_t skip_frames = 0) noexcept
  {
    if constexpr (is_ptr_v<V>) {
      detail::fail_range_pointer(msg, left, value, right, skip_frames + 1);
    } else if constexpr (is_signed_v<V>) {
      detail::fail_range_signed(msg, left, value, right, skip_frames + 1);
    } else {
      static_assert(is_integral_v<V>);
      detail::fail_range_unsigned(msg, left, value, right, skip_frames + 1);
    }
  }
}

#define J_SYS_ERR(...) ::j::exceptions::throw_system_error_exception(0, __VA_ARGS__)

#define J_THROW(...)                                                        \
  do {                                                                      \
    using namespace ::j::exceptions;                                        \
    ::j::exceptions::throw_exception(__VA_ARGS__);                          \
  } while (false)

#define J_FAIL(...) ::j::exceptions::fail_exception(__VA_ARGS__)

#define J_TODO(...) ::j::exceptions::fail_todo(J_VA_DEFAULTS(J_FUNC, __VA_ARGS__))

#ifndef NDEBUG
#define J_ASSERT(ASSERTION, ...) (J_LIKELY(ASSERTION) ? (void)0 \
                                  : ::j::exceptions::fail_logic_error_exception( \
                                    J_VA_DEFAULTS(#ASSERTION " failed in " __FILE_NAME__ ":" J_STRINGIFY(__LINE__), \
                                                  __VA_ARGS__)))

  #define J_ASSERT_EX(ASSERTION, ...)                                         \
    do {                                                                      \
      using namespace ::j::exceptions;                                        \
      if (!J_LIKELY(ASSERTION))                                               \
        ::j::exceptions::fail_logic_error_exception(__VA_ARGS__);            \
    } while (false)

  #define J_ASSERT_FAIL(...) ::j::exceptions::fail_exception(__VA_ARGS__)

  #define J_ASSUME(...) (J_LIKELY(__VA_ARGS__) ? __builtin_assume(__VA_ARGS__) \
    : ::j::exceptions::fail_logic_error_exception(#__VA_ARGS__ " failed in " __FILE_NAME__ ":" J_STRINGIFY(__LINE__)))

  #define J_ASSERT_SZ(SZ) (J_LIKELY(SZ > 0) ? (void)0 : ::j::exceptions::fail_out_of_range())
  #define J_ASSERT_RANGE_LT(I, SZ) (J_LIKELY(I < SZ) ? (void)0 : ::j::exceptions::fail_out_of_range(0, I, SZ))
  #define J_ASSERT_RANGE_LTE(I, SZ) (J_LIKELY(I <= SZ) ? (void)0 : ::j::exceptions::fail_out_of_range(0, I, SZ))

  #define J_ASSERT_RANGE_0_LT(I, SZ) (J_LIKELY((u32_t)(I) < (u32_t)(SZ)) ? (void)0 : ::j::exceptions::fail_out_of_range_0(I, SZ))
  #define J_ASSERT_RANGE_0_LTE(I, SZ) (J_LIKELY((u32_t)(I) <= (u32_t)(SZ)) ? (void)0 : ::j::exceptions::fail_out_of_range_0(I, SZ))

  #define J_ASSERT_RANGE(L, V, R) (J_LIKELY(((L) <= (V)) && ((V) < (R))) ? (void)0 \
    : ::j::exceptions::fail_range(#L " <= " #V " < " #R, (L), (V), (R)))

  #define J_ASSERT_NOT_NULL_BASE(VALUE)                                     \
    (J_LIKELY(VALUE) ? (void)0 : ::j::exceptions::fail_not_null(#VALUE, __FILE_NAME__ ":" J_STRINGIFY(__LINE__)))

  #define J_ASSERT_NOT_NULL(...) (J_FEC(J_ASSERT_NOT_NULL_BASE, __VA_ARGS__))

  #define J_ASSUME_NOT_NULL_BASE(VALUE) (J_LIKELY(VALUE) ? __builtin_assume(VALUE) : ::j::exceptions::fail_not_null(#VALUE, __FILE_NAME__ ":" J_STRINGIFY(__LINE__)))

#else

  #define J_ASSUME(...)               __builtin_assume(__VA_ARGS__)
  #define J_ASSUME_NOT_NULL_BASE(VAR) __builtin_assume(VAR)
  #define J_ASSERT_FAIL(...)          J_UNREACHABLE()
  #define J_ASSERT(...)               do { } while(false)
  #define J_ASSERT_EX(...)            do { } while(false)
  #define J_ASSERT_RANGE(...)         do { } while(false)
  #define J_ASSERT_NOT_NULL(...)      do { } while (false)

#endif

#define J_REQUIRE(ASSERTION, ...)                                         \
  (J_LIKELY(ASSERTION) ? (void)0 : ::j::exceptions::throw_exception(__VA_ARGS__))

#define J_REQUIRE_EX(ASSERTION, ...)                                      \
  do {                                                                    \
    using namespace ::j::exceptions;                                      \
    if (J_UNLIKELY(!(ASSERTION)))                                         \
      ::j::exceptions::throw_exception(__VA_ARGS__);                      \
  } while (false)

#define J_REQUIRE_NOT_NULL_BASE(VALUE)                                    \
  if (!J_LIKELY(VALUE)) ::j::exceptions::throw_not_null(#VALUE);

#define J_REQUIRE_NOT_NULL(...) do { J_FE(J_REQUIRE_NOT_NULL_BASE, __VA_ARGS__) } while (false)

#define J_ASSUME_NOT_NULL(...) ( J_FEC(J_ASSUME_NOT_NULL_BASE, __VA_ARGS__) )
