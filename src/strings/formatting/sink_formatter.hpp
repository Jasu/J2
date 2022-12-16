#pragma once

#include "strings/formatting/format_value.hpp"
#include "strings/string_view.hpp"
#include "strings/styling/style.hpp"
#include "strings/concepts.hpp"

namespace j::strings {
  inline namespace styling {
    class styled_sink;
    struct style;
  }
  inline namespace formatting {
    struct placeholder;
    extern const placeholder g_empty_placeholder;

    /// Wraps a sink to allow formatting. Buffered.
    class sink_formatter {
    public:
      constexpr sink_formatter() noexcept = default;

      template<typename... Params>
      J_A(ND) u32_t write(styled_sink & sink, style current_style, const const_string_view & format_string, Params && ... params) noexcept {
        if constexpr (sizeof...(params) == 0U) {
          return this->format(sink, format_string, 0U, nullptr, current_style);
        } else {
          format_value values[]{format_value(static_cast<Params &&>(params))...};
          return this->format(sink, format_string, sizeof...(Params), values, current_style);
        }
      }

      template<typename Param, typename... Params>
      J_A(ND) inline u32_t write(styled_sink & sink, const_string_view format_string, Param && param, Params && ... params) noexcept {
        const format_value values[]{format_value(static_cast<Param &&>(param)), format_value(static_cast<Params &&>(params))...};
        return this->format(sink, format_string, sizeof...(Params) + 1U, values);
      }

      template<IsAnyString T>
      J_A(AI,ND) inline u32_t write(styled_sink & sink, const T & value) noexcept {
        return this->format(sink, value, 0U, nullptr);
      }

      u32_t write(styled_sink & sink, const_string_view value) noexcept;
      u32_t write(styled_sink & sink, const char * value) noexcept;

      template<NonArray T>
      inline u32_t write(styled_sink & sink, T && value) noexcept {
        return this->format_single(sink, format_value(static_cast<T &&>(value)));
      }

      u32_t format(styled_sink & sink,
                  const const_string_view & format_string,
                  u32_t num_params,
                  const format_value * params) noexcept;

      u32_t format(styled_sink & sink,
                  const const_string_view & format_string,
                  u32_t num_params,
                  const format_value * params,
                  style current_style) noexcept;
    private:
      u32_t format_single(styled_sink & sink,
                          const format_value & param,
                          style current_style = g_empty_style,
                          const formatting::placeholder & ph = g_empty_placeholder) noexcept;
    };
  }
}
