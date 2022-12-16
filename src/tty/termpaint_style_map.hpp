#pragma once

#include "tty/attr.hpp"

#include "util/singleton.hpp"
#include "strings/styling/style.hpp"

namespace j::tty {
  struct termpaint_style_map final {
    J_BOILERPLATE(termpaint_style_map, COPY_DEL)

    termpaint_style_map();
    ~termpaint_style_map();

    attr get_attr(const strings::styling::style & style) noexcept;
  private:
    struct termpaint_style_map_private * priv = nullptr;
  };

  extern constinit util::singleton<termpaint_style_map> style_map;
}
