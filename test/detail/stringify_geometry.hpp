#pragma once

#include "stringify.hpp"

#include "strings/format.hpp"

#include "geometry/vec4.hpp"
#include "geometry/rect.hpp"
#include "geometry/perimeter.hpp"

namespace j::geometry {
  template<typename T>
  J_A(AI,ND,HIDDEN) inline doctest::String toString(const vec2<T> & v) {
    return j::strings::toString(j::strings::format("{}", v));
  }
  template<typename T>
  J_A(AI,ND,HIDDEN) inline doctest::String toString(const vec4<T> & v) {
    return j::strings::toString(j::strings::format("{}", v));
  }
  template<typename T>
  J_A(AI,ND,HIDDEN) inline doctest::String toString(const rect<T> & v) {
    return j::strings::toString(j::strings::format("{}", v));
  }
  template<typename T>
  J_A(AI,ND,HIDDEN) inline doctest::String toString(const perimeter<T> & v) {
    return j::strings::toString(j::strings::format("{}", v));
  }
}
