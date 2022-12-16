#include "strings/parsing/parse_float.hpp"
#include "strings/string.hpp"
#include "hzd/math.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#include <string-to-double.h>
#pragma clang diagnostic pop

namespace j::strings {
  inline namespace parsing {
    namespace {
      namespace d = double_conversion;
      const d::StringToDoubleConverter g_converter{
        d::StringToDoubleConverter::NO_FLAGS,
        nan_v<double>,
        nan_v<double>,
        "Inf",
        "NaN"
      };
    }

    template<> float parse_float<float>(const char * str, u32_t sz) {
      int num_processed;
      return g_converter.StringToFloat(str, sz, &num_processed);
    }
    template<> float parse_float_s<float>(const string & string) {
      return parse_float<float>(string.data(), string.size());
    }
    template<> float parse_float_sv<float>(const const_string_view & string) {
      return parse_float<float>(string.data(), string.size());
    }

    template<> double parse_float<double>(const char * str, u32_t sz) {
      int num_processed;
      return g_converter.StringToDouble(str, sz, &num_processed);
    }
    template<> double parse_float_s<double>(const string & string) {
      return parse_float<double>(string.data(), string.size());
    }
    template<> double parse_float_sv<double>(const const_string_view & string) {
      return parse_float<double>(string.data(), string.size());
    }
  }
}
