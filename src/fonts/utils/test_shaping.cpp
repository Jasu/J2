#include "services/service_instance.hpp"
#include "cli/cli.hpp"
#include "fonts/rasterizing/font_rasterizer.hpp"
#include "fonts/rasterizing/font_image_source_handler.hpp"
#include "fonts/rasterizing/rasterizer_font.hpp"
#include "fonts/rasterizing/glyph_info.hpp"
#include "fonts/selection/font_selector.hpp"
#include "fonts/selection/font_specification.hpp"
#include "fonts/shaping/shaped_string.hpp"
#include "fonts/shaping/shaper.hpp"
#include "fonts/font_set.hpp"
#include "fonts/font_size.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "strings/string.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"

namespace j::fonts::utils {
  namespace {
    using namespace selection;
    using namespace shaping;
    using namespace rasterizing;

    void test_selector(mem::shared_ptr<font_selector> selector,
                       mem::shared_ptr<font_rasterizer> rasterizer,
                       mem::shared_ptr<font_image_source_handler> handler,
                       mem::shared_ptr<shaper> shaper,

                       mem::shared_ptr<strings::formatted_sink> stdout,

                       font_weight weight_,
                       font_slant slant_,
                       font_width width_,
                       font_spacing spacing_,
                       strings::string family_,
                       strings::string text)
    {
      namespace s = j::strings;
      font_specification spec{
        family = s::const_string_view(family_),
        weight = weight_,
        slant = slant_,
        width = width_,
        spacing = spacing_,
        size = font_size(pixel_size_tag, 16)
      };

      auto selection = selector->select_fonts(spec);
      auto shaped = shaper->shape(s::rope_utf8(text), selection, 72);

      auto rast = selection[0].rasterizer_font();

      stdout->write("{#bold}Font info:{/}\n"
                    "  Pixel size:          {}\n"
                    "  Line height:         {}\n"
                    "  Ascent:              {}\n"
                    "  Descent:             {}\n"
                    "  Underline position:  {}\n"
                    "  Underline thickness: {}\n\n\n",
                    rast->pixel_size,
                    rast->line_height,
                    rast->ascent,
                    rast->descent,
                    rast->underline_position,
                    rast->underline_thickness);

      for (auto g : *shaped) {
        auto info = rasterizer->get_glyph_info(glyph_key(handler->index(), shaped->font_index(), g.glyph_index));
        stdout->write("  Glyph #{#bright_yellow}{}{/} at {#bright_cyan}({}, {}){/} to {#bright_cyan}({}, {}){/}, Bearing {#bright_cyan}({}, {}){/}\n",
                      g.glyph_index,
                      g.x + info.x_bearing(),
                      g.y - info.y_bearing(),
                      g.x + info.x_bearing() + info.width(),
                      g.y - (info.y_bearing() + info.height()),
                      info.x_bearing(),
                      info.y_bearing());
      }
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "fonts.utils.test_shaping",
      "Selector test command",

      c::command_name = "test-shaping",
      c::callback     = s::call(&test_selector,
                                s::arg::autowire,
                                s::arg::autowire,
                                s::arg::autowire,
                                s::arg::autowire,

                                s::service("stdout"),

                                c::option("weight", c::default_value=font_weight::regular),
                                c::option("slant", c::default_value=font_slant::roman),
                                c::option("width", c::default_value=font_width::normal),
                                c::option("spacing", c::default_value=font_spacing::proportional),
                                c::option("family", c::default_value=strings::string("sans-serif")),
                                c::argument("text")));
  }
}
