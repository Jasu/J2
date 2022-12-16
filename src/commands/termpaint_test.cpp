#include "cli/cli.hpp"
#include "services/service_instance.hpp"
#include "tty/components/divider.hpp"
#include "tty/components/scroll_view.hpp"
#include "tty/components/static_string.hpp"
#include "tty/components/line_editor.hpp"
#include "tty/components/linear_layout.hpp"
#include "strings/styling/default_styles.hpp"
#include "events/event_loop.hpp"
#include "logging/global.hpp"
#include "tty/uv_termpaint_integration.hpp"

namespace j::commands {
  namespace {
    void termpaint_test(mem::shared_ptr<j::events::event_loop> event_loop) {
      logging::global_logger::instance.set_sink_to_file("debug.log");

      tty::uv_termpaint_integration uvi(event_loop, 0);
      uvi.initialize();

      tty::line_editor le(&uvi);
      le.set_text("");
      le.set_cursor_position(0);
      le.set_prompt_style(strings::styling::styles::bold);
      le.set_continuation_style(strings::styling::styles::light_gray);

      tty::static_string str("Hello, world!");


      tty::linear_layout lin(tty::axis::vertical);
      tty::divider divider;
      divider.set_style(strings::styling::styles::bright_magenta);

      lin.append_child(&str);
      lin.append_child(&divider);
      lin.append_child(&le);

      tty::scroll_view sv(tty::axis::vertical, tty::scroll_mode::bottom, &lin);

      uvi.set_root(&sv);
      event_loop->run();
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "commands.termpaint_test",
      "Termpaint test",

      c::command_name = "termpaint-test",
      c::callback     = s::call(&termpaint_test, s::arg::autowire));
  }
}
