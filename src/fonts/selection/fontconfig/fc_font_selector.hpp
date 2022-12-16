#pragma once

#include "fonts/selection/font_selector.hpp"

namespace j::fonts::selection::fontconfig {
  /// FontConfig font selector.
  class fc_font_selector final : public font_selector {
  public:
    fc_font_selector();

    ~fc_font_selector();

    font_set select_fonts(const font_specification & spec) const override;
  };
}
