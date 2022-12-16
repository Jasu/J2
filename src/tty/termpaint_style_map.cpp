#include "tty/termpaint_style_map.hpp"

#include "containers/hash_map.hpp"
#include "mem/weak_ptr.hpp"
#include "mem/shared_ptr.hpp"
#include "strings/styling/style_hash.hpp"

#include <termpaint.h>

namespace j::tty {
  struct termpaint_style_map_private final {
    hash_map<j::strings::styling::style, j::mem::weak_ptr<termpaint_attr_>, j::strings::styling::style_hash> map;
  };

  termpaint_style_map::termpaint_style_map()
    : priv(::new termpaint_style_map_private)
  {
  }

  termpaint_style_map::~termpaint_style_map() {
    ::delete priv;
  }


  attr termpaint_style_map::get_attr(const strings::styling::style & style) noexcept {
    auto res = priv->map.maybe_at(style);
    attr result = res ? res->lock() : attr{};
    if (result) {
      return result;
    }
    unsigned fg = TERMPAINT_DEFAULT_COLOR, bg = TERMPAINT_DEFAULT_COLOR;
    if (!style.foreground.empty()) {
      fg = TERMPAINT_RGB_COLOR(style.foreground.rgb.r, style.foreground.rgb.g, style.foreground.rgb.b);
    }
    if (!style.background.empty()) {
      bg = TERMPAINT_RGB_COLOR(style.background.rgb.r, style.background.rgb.g, style.background.rgb.b);
    }
    result = j::mem::wrap_shared<termpaint_attr_, termpaint_attr_free>(termpaint_attr_new(fg, bg));
    int style_bits = 0;
    if (style.flags.has(strings::style_flag::bold)) {
      style_bits |= TERMPAINT_STYLE_BOLD;
    }
    if (style.flags.has(strings::style_flag::italic)) {
      style_bits |= TERMPAINT_STYLE_ITALIC;
    }
    if (style.flags.has(strings::style_flag::underline)) {
      style_bits |= TERMPAINT_STYLE_UNDERLINE;
    }
    if (style.flags.has(strings::style_flag::strikethrough)) {
      style_bits |= TERMPAINT_STYLE_STRIKE;
    }
    if (style_bits) {
      termpaint_attr_set_style(result.get(), style_bits);
    }

    if (res) {
      *res = result;
    } else {
      priv->map.emplace(style, result);
    }
    return result;
  }

  J_A(NODESTROY) constinit util::singleton<termpaint_style_map> style_map;
}
