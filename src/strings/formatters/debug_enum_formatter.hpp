#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::strings::formatters {
  struct debug_enum_entry final {
    const char * name = nullptr;
    style style = {};
  };

  struct debug_enum_formatter_base {
    const debug_enum_entry * const ptr = nullptr;
    const u32_t size = 0U;

    [[nodiscard]] constexpr const debug_enum_entry * maybe_get_entry(u32_t val) const noexcept {
      if (val >= size || !ptr[val].name) {
        return nullptr;
      }
      return ptr + val;
    }

    [[nodiscard]] u32_t get_length(u32_t val, const strings::const_string_view & opts) const noexcept;

    void format(u32_t value, styled_sink & target, const strings::const_string_view & opts) const;
  };

  template<typename Enum>
  struct debug_enum_formatter final : formatter_known_length<Enum>, debug_enum_formatter_base {
    template<u32_t N>
    J_A(ND) inline debug_enum_formatter(const debug_enum_entry (& entries)[N], const char * name = nullptr) noexcept
      : formatter_known_length<Enum>(name, name ? false : true),
        debug_enum_formatter_base{entries, N}
    {
    }

    J_NO_DEBUG void do_format(
      const const_string_view & opts,
      const Enum & value,
      styled_sink & target,
      style
      ) const override {
      debug_enum_formatter_base::format(static_cast<u64_t>(value), target, opts);
    }

    [[nodiscard]] J_NO_DEBUG u32_t do_get_length(const const_string_view & opts, const Enum & value) const noexcept override {
      return debug_enum_formatter_base::get_length(static_cast<u64_t>(value), opts);
    }
  };
}
