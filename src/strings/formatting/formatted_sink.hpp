#pragma once

#include "strings/formatting/sink_formatter.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::streams {
  class sink;
}

namespace j::strings {
  inline namespace styling { class styled_sink; }
  inline namespace formatting {
    class formatted_sink {
    public:
      J_A(ND) inline formatted_sink() noexcept = default;

      explicit formatted_sink(const mem::shared_ptr<styling::styled_sink> & sink) noexcept;

      explicit formatted_sink(const mem::shared_ptr<streams::sink> & sink) noexcept;

      template<typename... Params>
      J_A(AI,ND) u32_t write(Params && ... params) noexcept {
        return formatter.write(*sink, static_cast<Params &&>(params)...);
      }
      u32_t write_styled(const style & style, const strings::const_string_view & str) noexcept;

      u32_t write_styled(const style & style, const char * J_NOT_NULL str) noexcept;

      u32_t write_unformatted(const_string_view s) noexcept;
      u32_t write_unformatted(const char * J_NOT_NULL s) noexcept;
      u32_t write_unformatted(styling::style style, const_string_view s) noexcept;

      void flush() noexcept;
      void reset_column() noexcept;
      void pad_to_column(i32_t col) noexcept;

      J_NO_DEBUG u32_t format(const const_string_view & format_string,
                             u32_t num_params,
                             const format_value * params) noexcept {
        return formatter.format(*sink, format_string, num_params, params);
      }

      J_NO_DEBUG u32_t format(const const_string_view & format_string, u32_t num_params,
                                  const format_value * params, styling::style current_style) noexcept {
        return formatter.format(*sink, format_string, num_params, params, current_style);
      }

      ~formatted_sink();

      [[nodiscard]] inline operator styling::styled_sink & () noexcept
      { return *sink; }

      J_INLINE_GETTER explicit operator bool() const noexcept
      { return (bool)sink; }
      J_INLINE_GETTER bool operator!() const noexcept
      { return !(bool)sink; }

      mem::shared_ptr<styling::styled_sink> sink;
      sink_formatter formatter;
    };
  }
}

J_DECLARE_EXTERN_SHARED_PTR(j::strings::formatted_sink);
