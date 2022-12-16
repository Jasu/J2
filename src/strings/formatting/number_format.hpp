#pragma once

#include "hzd/types.hpp"

namespace j::strings {
  class string;
  inline namespace formatting {
    string number_format_hex(u64_t number);

    string number_format(float number);
    string number_format(double number);

    string number_format(u64_t number);
    string number_format(u32_t number);
    string number_format(u16_t number) noexcept;
    string number_format(u8_t number) noexcept;

    string number_format(i64_t number);
    string number_format(i32_t number);
    string number_format(i16_t number) noexcept;
    string number_format(i8_t number) noexcept;
  }
}
