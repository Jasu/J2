#pragma once

#include "hzd/concepts.hpp"
#include "strings/string_view.hpp"
#include "strings/string.hpp"

namespace j::strings {
  template<typename T>
  concept IsStringView = DerivedFrom<const_string_view, T> || DerivedFrom<string_view, T>;

  template<typename T>
  concept IsAnyString = IsStringView<T> || __is_same(T, const char *) || __is_same(T, char *)
    || __is_same(T, string);

  template<typename T>
  concept IsNotAnyString = !IsAnyString<T>;

  template<typename T>
  concept IsArgAnyString = IsAnyString<decay_t<T>>;

  template<typename T>
  concept IsArgNotAnyString = !IsAnyString<decay_t<T>>;
}
