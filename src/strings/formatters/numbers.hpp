#pragma once

#include "strings/formatters/integers.hpp"
#include "strings/formatters/floats.hpp"

namespace j::strings::formatters {
  template<typename T>
  inline const formatter_known_length<T> & number_formatter_v = integer_formatter_v<T>;

  template<>
  inline const formatter_known_length<float> & number_formatter_v<float> = float_formatter_v<float>;
  template<>
  inline const formatter_known_length<double> & number_formatter_v<double> = float_formatter_v<double>;
}
