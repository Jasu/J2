#pragma once

#include <vulkan/vulkan.h>

#include "strings/formatting/format_value.hpp"
#include "logging/severity.hpp"

namespace j::strings {
  class string;
}

namespace j::rendering::vulkan::detail {
  [[noreturn]] void throw_exception(const char * message);
  [[noreturn]] void fail_function(const char *function_name, VkResult result);

  void log_warning(strings::const_string_view message);

  void log_error(strings::const_string_view message);

  void log_info(strings::const_string_view message);

  void log_notice(strings::const_string_view message);

  void log_debug(strings::const_string_view message);

  void do_log(logging::severity sev, strings::const_string_view, u32_t num_params, const strings::formatting::format_value * params);

  template<typename... Params>
  J_A(AI,ND) void log(logging::severity sev, strings::const_string_view message, Params && ... params) {
    if constexpr (sizeof...(Params) == 0) {
      do_log(sev, message, 0, nullptr);
    } else {
      const strings::formatting::format_value values[] = { strings::formatting::format_value(static_cast<Params &&>(params))... };
      do_log(sev, message, sizeof...(Params), values);
    }
  }

  J_ALWAYS_INLINE void call_checked(const char *function_name, VkResult result) {
    if (J_UNLIKELY(result != VK_SUCCESS)) {
      fail_function(function_name, result);
    }
  }
}

#define J_VK_CALL_CHECKED(FUNCTION, ...)                        \
  ::j::rendering::vulkan::detail::call_checked(#FUNCTION, FUNCTION(__VA_ARGS__))

#define J_VK_FAIL_FUNCTION(FUNCTION, VK_RESULT)                         \
  ::j::rendering::vulkan::detail::fail_function(#FUNCTION, VK_RESULT)

#ifndef NDEBUG

  #define J_VK_ASSERT(ASSERTION, ...)                                 \
    do {                                                              \
      if (J_UNLIKELY(!(ASSERTION)))                                   \
        ::j::rendering::vulkan::detail::throw_exception(__VA_ARGS__); \
    } while(false)

#else
  #define J_VK_ASSERT(...) do { } while (false)
#endif

#define J_VK_THROW(MESSAGE) ::j::rendering::vulkan::detail::throw_exception(MESSAGE)

#define J_VK_ERROR(...)   ::j::rendering::vulkan::detail::log(::j::logging::severity::error,   __VA_ARGS__)
#define J_VK_WARNING(...) ::j::rendering::vulkan::detail::log(::j::logging::severity::warning, __VA_ARGS__)
#define J_VK_NOTICE(...)  ::j::rendering::vulkan::detail::log(::j::logging::severity::notice,  __VA_ARGS__)
#define J_VK_INFO(...)    ::j::rendering::vulkan::detail::log(::j::logging::severity::info,    __VA_ARGS__)
#define J_VK_DEBUG(...)   ::j::rendering::vulkan::detail::log(::j::logging::severity::debug,   __VA_ARGS__)
