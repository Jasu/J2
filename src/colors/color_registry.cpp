#include "colors/color_registry.hpp"

#include "exceptions/assert_lite.hpp"
#include "colors/default_colors.hpp"
#include "strings/string_map.hpp"

template struct j::util::singleton<j::colors::color_registry>;

namespace j::colors {
  color_registry::color_registry()
  {
    const pair<const char *, color> colors[]{
      {"black",          colors::black},
      {"red",            colors::red},
      {"green",          colors::green},
      {"yellow",         colors::yellow},
      {"blue",           colors::blue},
      {"magenta",        colors::magenta},
      {"cyan",           colors::cyan},
      {"light_gray",     colors::light_gray},
      {"gray",           colors::gray},
      {"bright_red",     colors::bright_red},
      {"bright_green",   colors::bright_green},
      {"bright_yellow",  colors::bright_yellow},
      {"bright_blue",    colors::bright_blue},
      {"bright_magenta", colors::bright_magenta},
      {"bright_cyan",    colors::bright_cyan},
      {"white",          colors::white},

      {"bright_orange",  colors::bright_orange},
      {"orange",         colors::orange},
    };
    for (auto & p : colors) {
      m_colors.insert(p.first, p.second);
    }
  }

  void color_registry::register_color(strings::string && str, const color & color) {
    bool did_register = m_colors.emplace(static_cast<strings::string &&>(str), color).second;
    J_REQUIRE(did_register, "Tried to register the same color twice.");
  }

  void color_registry::register_color(const strings::string & str, const color & color) {
    bool did_register = m_colors.emplace(str, color).second;
    J_REQUIRE(did_register, "Tried to register the same color twice.");
  }

  bool color_registry::has_color(const strings::string & str) const noexcept {
    return m_colors.contains(str);
  }

  const color & color_registry::at(const strings::string & str) const {
    return m_colors.at(str);
  }

  J_A(NODESTROY) constinit util::singleton<color_registry> g_color_registry;
}
