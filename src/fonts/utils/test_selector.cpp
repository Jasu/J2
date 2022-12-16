#include "services/service_instance.hpp"
#include "cli/cli.hpp"
#include "fonts/selection/font_selector.hpp"
#include "fonts/selection/font_specification.hpp"
#include "fonts/rasterizing/font_rasterizer.hpp"
#include "fonts/font_set.hpp"
#include "fonts/font_size.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::fonts::utils {
  namespace {
    using namespace selection;
    using namespace rasterizing;
    int test_selector(mem::shared_ptr<strings::formatted_sink> stdout,
                      mem::shared_ptr<font_selector> selector,
                      mem::shared_ptr<font_rasterizer> rasterizer,

                      font_weight weight_,
                      font_slant slant_,
                      font_width width_,
                      font_spacing spacing_,
                      strings::string family_)
    {
      font_specification spec{
        family = strings::const_string_view(family_),
        weight = weight_,
        slant = slant_,
        width = width_,
        spacing = spacing_,
        size = font_size(pixel_size_tag, 16)
      };

      auto selection = selector->select_fonts(spec);
      for (auto & f : selection) {
        stdout->write("Loading rasterizer font {}...\n", *reinterpret_cast<const strings::string*>(f.selector_font().get()));
        rasterizer->get_font(f, 72);
      }

      return 0;
    }

    using namespace cli;
    using namespace services;
    J_A(ND, NODESTROY) auto cmd = service_instance<cli::command>(
      "fonts.utils.test_selector",
      "Selector test command",

      command_name        = "test-selector",
      callback            = call(&test_selector,
                                 service("stdout"),

                                 arg::autowire,
                                 arg::autowire,

                                 option("weight", default_value=font_weight::regular),
                                 option("slant", default_value=font_slant::roman),
                                 option("width", default_value=font_width::normal),
                                 option("spacing", default_value=font_spacing::proportional),
                                 option("family", default_value=strings::string("sans-serif"))));
  }
}
