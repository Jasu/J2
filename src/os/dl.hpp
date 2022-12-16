#pragma once

#include "strings/string_view.hpp"

namespace j::os {
  struct loaded_library final {
    strings::const_string_view soname = {};
    strings::const_string_view path = {};
    const void * loaded_at = nullptr;

    J_A(AI,ND,HIDDEN,NODISC) inline explicit operator bool() const noexcept {
      return loaded_at;
    }
    J_A(AI,ND,HIDDEN,NODISC) inline bool operator!() const noexcept {
      return !loaded_at;
    }
  };

  [[nodiscard]] loaded_library get_loaded_library(strings::const_string_view soname);
}
