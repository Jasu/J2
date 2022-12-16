#include "ui/text/paragraph_layout_engine.hpp"
#include "ui/text/line_table.hpp"
#include "cli/cli.hpp"
#include "fonts/selection/font_selector.hpp"
#include "fonts/selection/font_specification.hpp"
#include "fonts/shaping/shaped_string.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"

namespace j::ui::utils {
  namespace {
    namespace str = j::strings;
    namespace sel = j::fonts::selection;
    void test_paragraph_layout(mem::shared_ptr<paragraph_layout_engine> engine,
                               mem::shared_ptr<sel::font_selector> selector,
                               mem::shared_ptr<str::formatted_sink> stdout,
                               u32_t num_lines,
                               str::string text)
    {
      sel::font_specification spec{
        sel::family = "sans-serif",
        sel::weight = sel::font_weight::regular,
        sel::slant = sel::font_slant::roman,
        sel::width = sel::font_width::normal,
        sel::spacing = sel::font_spacing::proportional,
        sel::size = fonts::font_size(fonts::pixel_size_tag, 16)
      };
      auto fonts = selector->select_fonts(spec);

      str::rope_utf8 rope(text);
      line_table tbl;
      engine->initialize_line_table(tbl, 0, num_lines, fonts, rope);
      stdout->write("{#bold,bright_green}Initialized line table:{/}\n");
      auto lineno = tbl.begin_line();
      for (auto & l : tbl) {
        stdout->write("  {#bold,bright_yellow}Line{/} #{}, size = {}:\n", lineno++, l.physical_line.size());
        u32_t i = 0U;
        for (auto & w : l.physical_line) {
          stdout->write("    {#bold,bright_magenta}Word{/} #{} {#bold,bright_cyan}({}x{}):\n", i++, w->width(), w->height());
          u32_t j = 0U;
          for (auto & c : *w) {
            stdout->write("      {#bold}Char{/} #{} Glyph={}, at=({}, {}):\n", j++, c.glyph_index, c.x, c.y);
          }
        }
      }

      engine->format_line_table(tbl, { 200.0f });

      stdout->write("{#bold,bright_green}Formatted line table:{/}\n");
      lineno = tbl.begin_line();
      for (auto & l : tbl) {
        stdout->write("  {#bold,bright_yellow}Line{/} #{}, {} visual lines:\n", lineno++, l.visual_lines.size());
        u32_t i = 0U;
        for (auto & vl : l.visual_lines) {
          stdout->write("    {#bold,bright_magenta}Visual line{/} #{} H={} Gap={}:\n", i++, vl.height, vl.gap);
          u32_t j = 0U;
          for (auto & w : vl.words) {
            stdout->write("      {#bold}Word{/} #{} at ({}, {})\n", j++, w.x, w.y);
          }
        }
      }
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "ui.utils.test_paragraph_layout",
      "Paragraph layout test command",

      c::command_name = "test-paragraph-layout",
      c::callback     = s::call(&test_paragraph_layout,
                                s::arg::autowire,
                                s::arg::autowire,
                                s::service("stdout"),
                                c::option("num-lines", c::default_value = 1U),
                                c::argument("text")));
  }
}
