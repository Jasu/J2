#include "properties/access/default_accesses.hpp"
#include "properties/access/float_access_registration.hpp"
#include "strings/string.hpp"
#include "strings/formatting/number_format.hpp"
#include "strings/parsing/parse_float.hpp"

namespace j::properties::access {
  namespace {
    template<typename To, typename From>
    J_INLINE_GETTER To identity_convert(From from) noexcept {
      return from;
    }

    float_access_registration<float> default_float_access_registration(
      "float",

      convertible_from = &identity_convert<float, double>,

      convertible_from = &identity_convert<float, i8_t>,
      convertible_from = &identity_convert<float, i16_t>,
      convertible_from = &identity_convert<float, i32_t>,
      convertible_from = &identity_convert<float, i64_t>,

      convertible_from = &identity_convert<float, u8_t>,
      convertible_from = &identity_convert<float, u16_t>,
      convertible_from = &identity_convert<float, u32_t>,
      convertible_from = &identity_convert<float, u64_t>,

      convertible_from = &strings::parse_float_s<float>,
      convertible_from = &strings::parse_float_sv<float>,

      convertible_to = (strings::string (*)(float))strings::number_format
    );
    float_access_registration<double> default_double_access_registration(
      "double",

      convertible_from = &identity_convert<double, float>,

      convertible_from = &identity_convert<double, i8_t>,
      convertible_from = &identity_convert<double, i16_t>,
      convertible_from = &identity_convert<double, i32_t>,
      convertible_from = &identity_convert<double, i64_t>,

      convertible_from = &identity_convert<double, u8_t>,
      convertible_from = &identity_convert<double, u16_t>,
      convertible_from = &identity_convert<double, u32_t>,
      convertible_from = &identity_convert<double, u64_t>,

      convertible_from = &strings::parse_float_s<double>,
      convertible_from = &strings::parse_float_sv<double>,

      convertible_to = (strings::string (*)(double))strings::number_format
    );
  }

  template<>
  const wrappers::wrapper_definition * wrapper_definition_v<float> = &float_access_registration<float>::wrapper;
  template<>
  const wrappers::wrapper_definition * wrapper_definition_v<double> = &float_access_registration<double>::wrapper;
  template<>
  const typed_access_definition * typed_access_definition_v<float> = &float_access_registration<float>::definition;
  template<>
  const typed_access_definition * typed_access_definition_v<double> = &float_access_registration<double>::definition;
}
