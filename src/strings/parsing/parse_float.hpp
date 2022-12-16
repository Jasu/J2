#pragma once

#include "hzd/types.hpp"

namespace j::strings {
  class string;
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
  inline namespace parsing {
    template<typename Float> Float parse_float(const char * str, u32_t sz);
    template<typename Float> Float parse_float_s(const string & string);
    template<typename Float> Float parse_float_sv(const const_string_view & string);

    template<> float parse_float<float>(const char * str, u32_t sz);
    template<> float parse_float_s<float>(const string & string);
    template<> float parse_float_sv<float>(const const_string_view & string);

    template<> double parse_float<double>(const char * str, u32_t sz);
    template<> double parse_float_s<double>(const string & string);
    template<> double parse_float_sv<double>(const const_string_view & string);
  }
}
