#pragma once

#include "strings/formatting/formatter.hpp"

namespace j::lisp::cir {
  struct loc;
}

namespace j::lisp::cir::inline locs {
  struct loc_mask;
  struct loc_specifier;
  extern const strings::formatter_known_length<loc> & g_loc_formatter;
  extern const strings::formatter_known_length<loc_mask> & g_loc_mask_formatter;
  extern const strings::formatter_known_length<loc_specifier> & g_loc_specifier_formatter;
}
