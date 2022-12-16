#pragma once

#include "strings/styling/styled_string.hpp"
#include "strings/formatting/format_value.hpp"

namespace j::strings {
  namespace detail {
    [[nodiscard]] styled_string format_styled(const const_string_view & format_string,
                                u32_t num_params,
                                const formatting::format_value * params);
    [[nodiscard]] string format(const const_string_view & format_string,
                                u32_t num_params,
                                const formatting::format_value * params);
  }

  template<typename... Params>
  [[nodiscard]] styled_string format_styled(const const_string_view & format_string, Params && ... params) {
    const formatting::format_value values[] = { formatting::format_value(static_cast<Params &&>(params))... };
    return detail::format_styled(format_string, sizeof...(Params), values);
  }

  [[nodiscard]] styled_string format_styled(const const_string_view & format_string);

  template<typename... Params>
  [[nodiscard]] inline string format(const const_string_view & format_string, Params && ... params) {
    const formatting::format_value values[]{formatting::format_value(static_cast<Params &&>(params))... };
    return detail::format(format_string, sizeof...(Params), values);
  }

  [[nodiscard]] string format(const const_string_view & format_string, u8_t param);
  [[nodiscard]] string format(const const_string_view & format_string, u16_t param);
  [[nodiscard]] string format(const const_string_view & format_string, u32_t param);
  [[nodiscard]] string format(const const_string_view & format_string, u64_t param);

  [[nodiscard]] string format(const const_string_view & format_string, i8_t param);
  [[nodiscard]] string format(const const_string_view & format_string, i16_t param);
  [[nodiscard]] string format(const const_string_view & format_string, i32_t param);
  [[nodiscard]] string format(const const_string_view & format_string, i64_t param);

  [[nodiscard]] string format(const const_string_view & format_string, const_string_view param);
  [[nodiscard]] string format(const const_string_view & format_string, const string & param);
  [[nodiscard]] string format(const const_string_view & format_string, const char * param);

  template<typename Param>
  [[nodiscard]] inline string format(const const_string_view & format_string, Param && param) {
    const formatting::format_value value{static_cast<Param &&>(param)};
    return detail::format(format_string, 1, &value);
  }

  [[nodiscard]] string format(const const_string_view & format_string);
}
