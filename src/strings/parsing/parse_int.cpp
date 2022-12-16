#include "strings/parsing/parse_int.hpp"
#include "strings/string.hpp"
#include "exceptions/exceptions.hpp"

namespace j::strings::inline parsing {
  namespace {
    [[noreturn]] J_NO_INLINE void throw_invalid_argument(const char * what) {
      throw exceptions::invalid_argument_exception() << exceptions::message(what);
    }

    template<typename Integer>
    [[nodiscard]] Integer parse_positive_sint(const char * str, u32_t sz) {
      Integer i = 0;
      while (sz) {
        int c = *str++ - '0';
        if (J_UNLIKELY(c < 0 || c > 9)) {
          throw_invalid_argument("Unexpected character in a signed integer.");
        }
        if (sz--) {
          if (J_UNLIKELY(__builtin_mul_overflow(i, (Integer)10, &i))) {
            throw_invalid_argument("Signed integer out of range.");
          }
        }
        if (J_UNLIKELY(__builtin_add_overflow(i, c, &i))) {
          throw_invalid_argument("Signed integer out of range.");
        }
      }
      return i;
    }

    template<typename Integer>
    [[nodiscard]] Integer parse_negative_sint(const char * str, u32_t sz) {
      Integer i = 0;
      while (sz) {
        int c = *str++ - '0';
        if (J_UNLIKELY(c < 0 || c > 9)) {
          throw_invalid_argument("Unexpected character in a signed integer.");
        }
        if (sz--) {
          if (J_UNLIKELY(__builtin_mul_overflow(i, (Integer)10, &i))) {
            throw_invalid_argument("Signed integer out of range.");
          }
        }
        if (J_UNLIKELY(__builtin_sub_overflow(i, c, &i))) {
          throw_invalid_argument("Signed integer out of range.");
        }
      }
      return i;
    }

    template<typename Integer>
    [[nodiscard]] Integer parse_sint(const char * str, u32_t sz) {
      if (str[0] == '-') {
        return parse_negative_sint<Integer>(str + 1, sz - 1);
      } else {
        return parse_positive_sint<Integer>(str + (str[0] == '+' ? 1U : 0U), sz - (str[0] == '+' ? 1U : 0U));
      }
    }

    template<typename Integer>
    [[nodiscard]] Integer parse_uint(const char * str, u32_t sz) {
      Integer i = 0;
      while (sz) {
        u8_t c = *str++ - '0';
        if (J_UNLIKELY(c > 9U)) {
          throw_invalid_argument("Unexpected character in an unsigned integer.");
        }
        if (sz--) {
          if (J_UNLIKELY(__builtin_mul_overflow(i, (Integer)10, &i))) {
            throw_invalid_argument("Unsigned integer out of range.");
          }
        }
        if (J_UNLIKELY(__builtin_add_overflow(i, c, &i))) {
          throw_invalid_argument("Unsigned integer out of range.");
        }
      }
      return i;
    }

    J_ALWAYS_INLINE void validate_uint_params(const char * str, u32_t sz) {
      if (J_UNLIKELY(!str || !sz || str[0] == 0)) {
        throw_invalid_argument("Null or empty strings cannot be parsed.");
      }
    }

    void validate_sint_params(const char * str, u32_t sz) {
      if (J_UNLIKELY(!str || !sz || str[0] == 0)) {
        throw_invalid_argument("Null or empty strings cannot be parsed.");
      }
      if (J_UNLIKELY((str[0] == '-' || str[0] == '+') && sz == 1)) {
        throw_invalid_argument("String cannot consist only of a unary plus or minus.");
      }
    }
  }

  template<>
  [[nodiscard]] u64_t parse_decimal_integer<u64_t>(const char * str, u32_t sz) {
    validate_uint_params(str, sz);
    return parse_uint<u64_t>(str, sz);
  }

  template<>
  [[nodiscard]] u32_t parse_decimal_integer<u32_t>(const char * str, u32_t sz) {
    validate_uint_params(str, sz);
    return parse_uint<u32_t>(str, sz);
  }

  template<>
  [[nodiscard]] u16_t parse_decimal_integer<u16_t>(const char * str, u32_t sz) {
    validate_uint_params(str, sz);
    return parse_uint<u16_t>(str, sz);
  }

  template<>
  [[nodiscard]] u8_t parse_decimal_integer<u8_t>(const char * str, u32_t sz) {
    validate_uint_params(str, sz);
    return parse_uint<u8_t>(str, sz);
  }

  template<>
  [[nodiscard]] i64_t parse_decimal_integer<i64_t>(const char * str, u32_t sz) {
    validate_sint_params(str, sz);
    return parse_sint<i64_t>(str, sz);
  }

  template<>
  [[nodiscard]] i32_t parse_decimal_integer<i32_t>(const char * str, u32_t sz) {
    validate_sint_params(str, sz);
    return parse_sint<i32_t>(str, sz);
  }

  template<>
  [[nodiscard]] i16_t parse_decimal_integer<i16_t>(const char * str, u32_t sz) {
    validate_sint_params(str, sz);
    return parse_sint<i16_t>(str, sz);
  }

  template<>
  [[nodiscard]] i8_t parse_decimal_integer<i8_t>(const char * str, u32_t sz) {
    validate_sint_params(str, sz);
    return parse_sint<i8_t>(str, sz);
  }

  template<>
  [[nodiscard]] u64_t parse_decimal_integer_s<u64_t>(const string & string) {
    return parse_decimal_integer<u64_t>(string.data(), string.size());
  }

  template<>
  [[nodiscard]] u32_t parse_decimal_integer_s<u32_t>(const string & string) {
    return parse_decimal_integer<u32_t>(string.data(), string.size());
  }

  template<>
  [[nodiscard]] u16_t parse_decimal_integer_s<u16_t>(const string & string) {
    return parse_decimal_integer<u16_t>(string.data(), string.size());
  }

  template<>
  [[nodiscard]] u8_t parse_decimal_integer_s<u8_t>(const string & string) {
    return parse_decimal_integer<u8_t>(string.data(), string.size());
  }

  template<>
  i64_t parse_decimal_integer_s<i64_t>(const string & string) {
    return parse_decimal_integer<i64_t>(string.data(), string.size());
  }

  template<> i32_t parse_decimal_integer_s<i32_t>(const string & string) {
    return parse_decimal_integer<i32_t>(string.data(), string.size());
  }

  template<> i16_t parse_decimal_integer_s<i16_t>(const string & string) {
    return parse_decimal_integer<i16_t>(string.data(), string.size());
  }

  template<> i8_t parse_decimal_integer_s<i8_t>(const string & string) {
    return parse_decimal_integer<i8_t>(string.data(), string.size());
  }

  template<> u64_t parse_decimal_integer_sv<u64_t>(const const_string_view & string) {
    return parse_decimal_integer<u64_t>(string.data(), string.size());
  }

  template<> u32_t parse_decimal_integer_sv<u32_t>(const const_string_view & string) {
    return parse_decimal_integer<u32_t>(string.data(), string.size());
  }

  template<> u16_t parse_decimal_integer_sv<u16_t>(const const_string_view & string) {
    return parse_decimal_integer<u16_t>(string.data(), string.size());
  }

  template<> u8_t parse_decimal_integer_sv<u8_t>(const const_string_view & string) {
    return parse_decimal_integer<u8_t>(string.data(), string.size());
  }

  template<> i64_t parse_decimal_integer_sv<i64_t>(const const_string_view & string) {
    return parse_decimal_integer<i64_t>(string.data(), string.size());
  }

  template<> i32_t parse_decimal_integer_sv<i32_t>(const const_string_view & string) {
    return parse_decimal_integer<i32_t>(string.data(), string.size());
  }

  template<> i16_t parse_decimal_integer_sv<i16_t>(const const_string_view & string) {
    return parse_decimal_integer<i16_t>(string.data(), string.size());
  }

  template<> i8_t parse_decimal_integer_sv<i8_t>(const const_string_view & string) {
    return parse_decimal_integer<i8_t>(string.data(), string.size());
  }
}
