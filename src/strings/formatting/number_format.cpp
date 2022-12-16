#include "strings/formatting/number_format.hpp"
#include "strings/string.hpp"
#include "strings/formatting/format_digits.hpp"
#include "hzd/type_traits.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#include <double-to-string.h>
#pragma clang diagnostic pop

namespace j::strings {
  inline namespace formatting {
    namespace d = double_conversion;
    namespace {
      const d::DoubleToStringConverter g_converter{
        d::DoubleToStringConverter::NO_FLAGS,
        "Inf",
        "NaN",
        'e',
        -7,
        22,
        6,
        6
      };

      template<typename T>
      J_INLINE_GETTER strings::string format_int(T number) noexcept(sizeof(T) <= 2) {
        u8_t digits = formatting::num_decimal_digits(number);
        strings::string result(digits);
        char *data = result.data();
        if constexpr (is_signed_v<T>) {
          if (number < 0) {
            *data++ = '-';
            number = -number;
            --digits;
          }
        }
        formatting::format_dec(number, data, digits);
        return result;
      }
    }

    strings::string number_format_hex(u64_t number) {
      const u8_t digits = formatting::num_hex_digits(number);
      strings::string result(digits);
      formatting::format_hex(number, result.data(), digits);
      return result;
    }

    strings::string number_format(i64_t number) {
      return format_int(number);
    }

    strings::string number_format(i32_t number) {
      return format_int(number);
    }

    strings::string number_format(i16_t number) noexcept {
      return format_int(number);
    }

    strings::string number_format(i8_t number) noexcept {
      return format_int(number);
    }

    strings::string number_format(u64_t number) {
      return format_int(number);
    }

    strings::string number_format(u32_t number) {
      return format_int(number);
    }

    strings::string number_format(u16_t number) noexcept {
      return format_int(number);
    }

    strings::string number_format(u8_t number) noexcept {
      return format_int(number);
    }

    strings::string number_format(double number) {
      char scratch[25];
      d::StringBuilder b(scratch, 25);
      g_converter.ToShortest(number, &b);
      return strings::string(scratch, b.position());
    }

    strings::string number_format(float number) {
      char scratch[25];
      d::StringBuilder b(scratch, 25);
      g_converter.ToShortestSingle(number, &b);
      return strings::string(scratch, b.position());
    }
  }
}
