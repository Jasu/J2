#pragma once

#include "strings/concepts.hpp"
#include "hzd/concepts.hpp"
#include "hzd/type_traits.hpp"
#include "mem/any.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::exceptions {
  J_A(NORET) void throw_logic_error_exception_fmt(const char * msg, u32_t sz, mem::any * anys, i32_t skip_frames = 0);
  J_A(NORET) void fail_logic_error_exception_fmt(const char * msg, u32_t sz, mem::any * anys, i32_t skip_frames = 0) noexcept;

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_any(mem::any && index, i32_t skip_frames = 0);

  template<typename T> requires strings::IsArgNotAnyString<T> && NotIntegral<T>
  J_A(ND,NORET) inline void throw_out_of_range(i32_t skip_frames, const T & index) {
    throw_out_of_range_any(mem::any(index), skip_frames + 1);
  }

  template<typename... Ts>
  J_A(ND,NI,NORET) inline void throw_logic_error_exception(const char * msg, Ts && ... ts) {
    mem::any anys[] = { mem::any(static_cast<Ts &&>(ts))... };
    throw_logic_error_exception_fmt(msg, sizeof...(Ts), anys, 1);
  }

  template<typename... Ts>
  J_A(ND,NI,NORET) inline void fail_logic_error_exception(const char * msg, Ts && ... ts) noexcept {
    mem::any anys[] = { mem::any(static_cast<Ts &&>(ts))... };
    fail_logic_error_exception_fmt(msg, sizeof...(Ts), anys, 1);
  }

  /// Throws [j::exceptions::exception] with the message specified.
  J_A(NORET) void throw_exception_fmt(const char * msg, u32_t sz, mem::any * anys, i32_t skip_frames = 0);
  /// Throws [j::exceptions::exception] with the message specified.
  J_A(NORET) void fail_exception_fmt(const char * msg, u32_t sz, mem::any * anys, i32_t skip_frames = 0) noexcept;

  template<typename... Ts>
  J_A(ND,NI,NORET) inline void throw_exception(const char * msg, Ts && ... ts) {
    mem::any anys[] = { mem::any(static_cast<Ts &&>(ts))... };
    throw_exception_fmt(msg, sizeof...(Ts), anys, 1);
  }

  template<typename... Ts>
  J_A(ND,NI,NORET) void fail_exception(const char * msg, Ts && ... ts) noexcept{
    mem::any anys[] = { mem::any(static_cast<Ts &&>(ts))... };
    fail_exception_fmt(msg, sizeof...(Ts), anys, 1);
  }
}
