#include "fonts/selection/fontconfig/fc_font_selector.hpp"

#include "exceptions/assert_lite.hpp"
#include "services/service_definition.hpp"
#include "files/paths/path.hpp"
#include "hzd/string.hpp"

#include <fontconfig/fontconfig.h>

namespace j::fonts::selection::fontconfig {
  namespace {
    int slant_to_fc(font_slant s) {
      switch (s) {
      case font_slant::roman:
        return FC_SLANT_ROMAN;
      case font_slant::oblique:
        return FC_SLANT_OBLIQUE;
      case font_slant::italic:
        return FC_SLANT_ITALIC;
      }
    }

    int weight_to_fc(font_weight s) {
      switch (s) {
      case font_weight::thin:
        return FC_WEIGHT_THIN;
      case font_weight::extra_light:
        return FC_WEIGHT_EXTRALIGHT;
      case font_weight::light:
        return FC_WEIGHT_LIGHT;
      case font_weight::demilight:
        return FC_WEIGHT_DEMILIGHT;
      case font_weight::book:
        return FC_WEIGHT_BOOK;
      case font_weight::regular:
        return FC_WEIGHT_REGULAR;
      case font_weight::medium:
        return FC_WEIGHT_MEDIUM;
      case font_weight::demibold:
        return FC_WEIGHT_DEMIBOLD;
      case font_weight::bold:
        return FC_WEIGHT_BOLD;
      case font_weight::extra_bold:
        return FC_WEIGHT_EXTRABOLD;
      case font_weight::black:
        return FC_WEIGHT_BLACK;
      case font_weight::extra_black:
        return FC_WEIGHT_EXTRABLACK;
      }
    }

    int spacing_to_fc(font_spacing s) {
      switch (s) {
      case font_spacing::proportional:
        return FC_PROPORTIONAL;
      case font_spacing::monospace:
        return FC_MONO;
      case font_spacing::dual_width:
        return FC_DUAL;
      case font_spacing::char_cell:
        return FC_CHARCELL;
      }
    }
    int width_to_fc(font_width s) {
      switch (s) {
      case font_width::ultra_condensed:
        return FC_WIDTH_ULTRACONDENSED;
      case font_width::extra_condensed:
        return FC_WIDTH_EXTRACONDENSED;
      case font_width::condensed:
        return FC_WIDTH_CONDENSED;
      case font_width::semicondensed:
        return FC_WIDTH_SEMICONDENSED;
      case font_width::normal:
        return FC_WIDTH_NORMAL;
      case font_width::semiexpanded:
        return FC_WIDTH_SEMIEXPANDED;
      case font_width::expanded:
        return FC_WIDTH_EXPANDED;
      case font_width::extra_expanded:
        return FC_WIDTH_EXTRAEXPANDED;
      case font_width::ultra_expanded:
        return FC_WIDTH_ULTRAEXPANDED;
      }
    }
  }

  fc_font_selector::fc_font_selector() {
    J_REQUIRE(FcInit(), "Initializing FontConfig failed.");
  }

  fc_font_selector::~fc_font_selector() {
    FcFini();
  }

  font_set fc_font_selector::select_fonts(const font_specification & spec) const {
    FcFontSet * fonts = nullptr;
    FcPattern * pattern = FcPatternCreate();
    J_REQUIRE_NOT_NULL(pattern);
    try {
      FcValue v;
      v.type = FcTypeBool;
      v.u.b = FcTrue;
      FcPatternAdd(pattern, FC_SCALABLE, v, FcFalse);

      v.type = FcTypeString;
      for (auto & f : spec.family) {
        strings::string terminated_string{f.size() + 1};
        memcpy(terminated_string.data(), f.data(), f.size());
        terminated_string[f.size()] = 0;
        v.u.s = reinterpret_cast<const FcChar8*>(terminated_string.data());
        FcPatternAdd(pattern, FC_FAMILY, v, FcTrue);
      }

      v.type = FcTypeInteger;
      if (spec.weight.empty()) {
        v.u.i = FC_WEIGHT_NORMAL;
        FcPatternAdd(pattern, FC_WEIGHT, v, FcTrue);
      } else {
        for (auto weight : spec.weight) {
          v.u.i = weight_to_fc(weight);
          FcPatternAdd(pattern, FC_WEIGHT, v, FcTrue);
        }
      }

      if (spec.slant.empty()) {
        v.u.i = FC_SLANT_ROMAN;
        FcPatternAdd(pattern, FC_SLANT, v, FcTrue);
      } else {
        for (auto slant : spec.slant) {
          v.u.i = slant_to_fc(slant);
          FcPatternAdd(pattern, FC_SLANT, v, FcTrue);
        }
      }

      if (!spec.width.empty()) {
        for (auto width : spec.width) {
          v.u.i = width_to_fc(width);
          FcPatternAdd(pattern, FC_WIDTH, v, FcTrue);
        }
      }

      if (!spec.spacing.empty()) {
        for (auto spacing : spec.spacing) {
          v.u.i = spacing_to_fc(spacing);
          FcPatternAdd(pattern, FC_SPACING, v, FcTrue);
        }
      }

      J_REQUIRE(FcConfigSubstitute(nullptr, pattern, FcMatchPattern),
                "Could not substitute into a FontConfig pattern.");
      FcDefaultSubstitute(pattern);

      FcResult result;
      fonts = FcFontSort(nullptr, pattern, FcTrue, nullptr, &result);
    } catch (...) {
      FcPatternDestroy(pattern);
      throw;
    }
    FcPatternDestroy(pattern);
    font_set result_fonts;
    try {
      J_REQUIRE_NOT_NULL(fonts, fonts->nfont);

      result_fonts.reserve(fonts->nfont);
      FcChar8 * path;
      for (int i = 0; i < fonts->nfont; ++i) {
        if (FcPatternGetString(fonts->fonts[i], FC_FILE, 0, &path) != FcResultMatch) {
          continue;
        }
        J_ASSERT_NOT_NULL(path);
        result_fonts.emplace_back(mem::make_void_shared<files::path>((const char*)path), spec.size);
      }
    } catch (...) {
      FcFontSetDestroy(fonts);
      throw;
    }
    FcFontSetDestroy(fonts);
    return result_fonts;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<fc_font_selector> def(
      "fonts.selector.fontconfig",
      "FontConfig font selector",
      s::implements = s::interface<font_selector>,
      s::global_singleton);
  }
}
