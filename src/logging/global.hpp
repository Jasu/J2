#pragma once

#include "logging/logging.hpp"

namespace j::logging {
  struct global_logger final : public logger {
    global_logger() noexcept;
    void initialize(mem::shared_ptr<strings::formatted_sink> sink);
    J_A(ND) static global_logger instance;
  };
}

#define J_LOG_SCOPED(...) \
  auto J_UNIQUE(logctx) = ([&]() { \
    using namespace j::logging::attributes; \
    using namespace j::strings::attributes; \
    using namespace j::strings::formatting; \
    return ::j::logging::global_logger::instance.begin(__VA_ARGS__); \
  })()

#define J_LOG_RESET_COL() ::j::logging::global_logger::instance.reset_column()
#define J_LOG_PAD_TO_COL(...) ::j::logging::global_logger::instance.pad_to_column(__VA_ARGS__)
#define J_LOG_FLUSH() ::j::logging::global_logger::instance.flush()
#define J_DEBUG(...) ::j::logging::global_logger::instance.log(::j::logging::severity::debug, true, __VA_ARGS__)
#define J_INFO(...) ::j::logging::global_logger::instance.log(::j::logging::severity::info, true, __VA_ARGS__)
#define J_NOTICE(...) ::j::logging::global_logger::instance.log(::j::logging::severity::notice, true, __VA_ARGS__)
#define J_WARNING(...) ::j::logging::global_logger::instance.log(::j::logging::severity::warning, true, __VA_ARGS__)
#define J_ERROR(...) ::j::logging::global_logger::instance.log(::j::logging::severity::error, true, __VA_ARGS__)
#define J_CRITICAL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::critical, true, __VA_ARGS__)

#define J_DEBUG_INL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::debug, false, __VA_ARGS__)
#define J_INFO_INL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::info, false, __VA_ARGS__)
#define J_NOTICE_INL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::notice, false, __VA_ARGS__)
#define J_WARNING_INL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::warning, false, __VA_ARGS__)
#define J_ERROR_INL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::error, false, __VA_ARGS__)
#define J_CRITICAL_INL(...) ::j::logging::global_logger::instance.log(::j::logging::severity::critical, false, __VA_ARGS__)

#define J_DEBUG_IF(COND, ...)     do { if (COND) { \
      ::j::logging::global_logger::instance.log(::j::logging::severity::debug, true, __VA_ARGS__); \
    } } while (false)
#define J_INFO_IF(COND, ...)      do { if (COND) { \
      ::j::logging::global_logger::instance.log(::j::logging::severity::info, true, __VA_ARGS__); \
    } } while (false)
#define J_NOTICE_IF(COND, ...)    do { if (J_UNLIKELY(COND)) { \
      ::j::logging::global_logger::instance.log(::j::logging::severity::notice, true, __VA_ARGS__); \
    } } while (false)
#define J_WARNING_IF(COND, ...)   do { if (J_UNLIKELY(COND)) { \
      ::j::logging::global_logger::instance.log(::j::logging::severity::warning, true, __VA_ARGS__); \
    } } while (false)
#define J_ERROR_IF(COND, ...)     do { if (J_UNLIKELY(COND)) { \
      ::j::logging::global_logger::instance.log(::j::logging::severity::error, true, __VA_ARGS__); \
    } } while (false)
#define J_CRITICAL_IF(COND, ...)  do { if (J_UNLIKELY(COND)) {          \
      ::j::logging::global_logger::instance.log(::j::logging::severity::critical, true, __VA_ARGS__); \
    } } while (false)
