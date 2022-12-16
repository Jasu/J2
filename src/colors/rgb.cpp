#include "colors/rgb.hpp"
#include "properties/class_registration.hpp"
#include "properties/assert.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/int_access.hpp"
#include "util/hex.hpp"

namespace j::colors {
  namespace {
    using namespace properties;

    [[noreturn]] J_NO_INLINE void throw_type_error() {
      throw_invalid_conversion("RGB color list must either be all-float or all-in...t.");
    }

    rgb8 from_int_list(const list_access & list_) {
      list_access list = list_;
      if (!list.at(1).is_int() || !list.at(2).is_int()) {
        throw_type_error();
      }
      auto r = list.at(0).as_int().get_value(),
        g = list.at(1).as_int().get_value(),
        b = list.at(2).as_int().get_value();
      if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        throw_invalid_conversion("RGB colors must be in range [0, 255] when specified as integers.");
      }
      return { (u8_t)r, (u8_t)g, (u8_t)b };
    }

    rgb8 from_float_list(const list_access & list_) {
      list_access list = list_;
      if (!list.at(1).is_float() || !list.at(2).is_float()) {
        throw_type_error();
      }
      auto r = list.at(0).as_float().get_value(),
        g = list.at(1).as_float().get_value(),
        b = list.at(2).as_float().get_value();
      if (r < 0.0 || r > 1.0 || g < 0.0 || g > 1.0 || b < 0.0 || b > 1.0) {
        throw_invalid_conversion("RGB colors must be in range [0.0, 1.0] when specified as floating-point numbers.");
      }
      return { (u8_t)(r * 255.0), (u8_t)(g * 255.0), (u8_t)(b * 255.0) };
    }

    rgb8 from_list(const list_access & list_) {
      list_access list = list_;
      if (list.size() != 3) {
        throw_invalid_conversion("RGB color list must have exactly three items.");
      }
      if (list.at(0).is_int()) {
        return from_int_list(list);
      } else if (list.at(0).is_float()) {
        return from_float_list(list);
      } else {
        throw_type_error();
      }
    }

    [[noreturn]] J_NO_INLINE void throw_string_error() {
      throw_invalid_conversion("RGB color must be either in \"#FFAA00\" or \"#fa0\" formats.");
    }

    rgb8 from_string(strings::const_string_view str) {
      if ((str.size() != 7 && str.size() != 4) || str[0] != '#') {
        throw_string_error();
      }
      str.remove_prefix(1);
      if (!util::are_hex_digits(str)) {
        throw_string_error();
      }
      if (str.size() == 6) {
        return { util::convert_hex_byte(str.data()),
                 util::convert_hex_byte(str.data() + 2),
                 util::convert_hex_byte(str.data() + 4) };
      }
      u8_t r = util::convert_hex_digit(str[0]),
        g = util::convert_hex_digit(str[1]),
        b = util::convert_hex_digit(str[2]);
      return {
        static_cast<u8_t>(r | (u8_t)(r << 4)),
        static_cast<u8_t>(g | (u8_t)(g << 4)),
        static_cast<u8_t>(b | (u8_t)(b << 4)) };
    }

    object_access_registration<rgb8> rgb_access_registration{
      "rgb8",

      property = member<&rgb8::r>("r"),
      property = member<&rgb8::g>("g"),
      property = member<&rgb8::b>("b"),

      convertible_from = &from_string,
      convertible_from = &from_list,
    };
  }

  const typed_access_definition & rgb_access_definition = object_access_registration<rgb8>::definition;
}
