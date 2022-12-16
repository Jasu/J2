#pragma once

#include "strings/styling/style.hpp"

namespace j::strings::formatters {
  struct enum_value final {
    template<typename Enum>
    J_ALWAYS_INLINE constexpr enum_value(Enum value,
                                         const char * J_NOT_NULL name,
                                         style s = {}) noexcept
      : value(static_cast<u64_t>(value)),
        name(name),
        style(s)
    { }

    u64_t value;
    const char * name = nullptr;
    style style;
  };
}
