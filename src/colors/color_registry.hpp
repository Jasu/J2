#pragma once

#include "colors/color.hpp"
#include "strings/string_map_fwd.hpp"
#include "util/singleton.hpp"

namespace j::colors {
  class color_registry final {
  public:
    color_registry();

    void register_color(strings::string && str, const color & color);
    void register_color(const strings::string & str, const color & color);
    bool has_color(const strings::string & str) const noexcept;
    const color & at(const strings::string & str) const;
  private:
    J_HIDDEN strings::string_map<color> m_colors;
  };

  extern constinit util::singleton<color_registry> g_color_registry;
}
extern template struct j::util::singleton<j::colors::color_registry>;
