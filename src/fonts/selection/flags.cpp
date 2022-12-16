#include "fonts/selection/flags.hpp"

#include "properties/enum_registration.hpp"

namespace j::fonts::selection {
  namespace {
    using namespace j::properties;

    enum_access_registration<font_weight> weight_reg(
      "font_weight",

      value = enum_value(font_weight::thin, "thin"),
      value = enum_value(font_weight::extra_light, "extra_light"),
      value = enum_value(font_weight::light, "light"),
      value = enum_value(font_weight::demilight, "demilight"),
      value = enum_value(font_weight::book, "book"),
      value = enum_value(font_weight::regular, "regular"),
      value = enum_value(font_weight::medium, "medium"),
      value = enum_value(font_weight::bold, "bold"),
      value = enum_value(font_weight::extra_bold, "extra_bold"),
      value = enum_value(font_weight::black, "black"),
      value = enum_value(font_weight::extra_black, "extra_black")
    );

    enum_access_registration<font_slant> slant_reg(
      "font_slant",

      value = enum_value(font_slant::roman, "roman"),
      value = enum_value(font_slant::oblique, "oblique"),
      value = enum_value(font_slant::italic, "italic")
    );

    enum_access_registration<font_width> width_reg(
      "font_width",

      value = enum_value(font_width::ultra_condensed, "ultra_condensed"),
      value = enum_value(font_width::extra_condensed, "extra_condensed"),
      value = enum_value(font_width::condensed, "condensed"),
      value = enum_value(font_width::semicondensed, "semicondensed"),
      value = enum_value(font_width::normal, "normal"),
      value = enum_value(font_width::semiexpanded, "semiexpanded"),
      value = enum_value(font_width::expanded, "expanded"),
      value = enum_value(font_width::extra_expanded, "extra_expanded"),
      value = enum_value(font_width::ultra_expanded, "ultra_expanded")
    );

    enum_access_registration<font_spacing> spacing_reg(
      "font_spacing",

      value = enum_value(font_spacing::proportional, "proportional"),
      value = enum_value(font_spacing::monospace, "monospace"),
      value = enum_value(font_spacing::dual_width, "dual_width"),
      value = enum_value(font_spacing::char_cell, "char_cell")
    );

    // set_access_registration<font_flags::field_access_t<font_weight>>
    //   weight_mask_reg("font_flags::weight");
    // set_access_registration<font_flags::field_access_t<font_slant>>
    //   slant_mask_reg("font_flags::slant");
    // set_access_registration<font_flags::field_access_t<font_spacing>>
    //   spacing_mask_reg("font_flags::spacing");
    // set_access_registration<font_flags::field_access_t<font_width>>
    //   width_mask_reg("font_flags::width");
  }
}
