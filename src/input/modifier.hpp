#pragma once

#include "util/flags.hpp"

namespace j::input {
  /// Named modifier keys
  enum class modifier : u8_t {
    /// Normal (level 2) shift
    shift,
    /// Control
    ctrl,
    /// Alt / Meta / Option
    alt,
    /// "Windows" button / Mod4
    super,
    /// Alternate graphic / ISO Level 3 shift
    alt_gr,
    /// Caps lock (separate from shift)
    caps_lock,
  };

  inline constexpr modifier g_modifiers[]{
    modifier::shift, modifier::ctrl, modifier::alt,
    modifier::super, modifier::alt_gr, modifier::caps_lock};

  using modifier_mask = util::flags<modifier, u8_t>;
  J_FLAG_OPERATORS(modifier, u8_t)
}
