#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::strings::formatters {
  template<typename T> extern const formatter_known_length<T> & float_formatter_v;
  template<> extern const formatter_known_length<float> & float_formatter_v<float>;
  template<> extern const formatter_known_length<double> & float_formatter_v<double>;
}
