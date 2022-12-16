#include "exceptions/exceptions.hpp"
#include "exceptions/formatters.hpp"
#include "services/services.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "tty/tty_restore.hpp"

#include <exception>

namespace j::exceptions {
  void handle_terminate() {
    mem::shared_ptr<strings::formatted_sink> stderr_sink = j::services::service_container->get<strings::formatted_sink>(services::service_reference("stderr"));

    tty::restore_tty_mode();
    try {
      auto e = std::current_exception();
      if (!e) {
        stderr_sink->write("{#error}Error in error handling:{/}\n"
                           "{#bright_red,bold}Terminate called without an exception.{/}");
        return;
      }
      std::rethrow_exception(e);
    } catch (exception & e) {
      j::services::service_container->get<exception_formatter>()->format(*stderr_sink, e);
    } catch (std::exception & e) {
      stderr_sink->write("{#error}std::exception:{/}\n"
                         "{#bright_red,bold}{}.{/}",
                         e.what());
    }
  }

  namespace {
    class J_TYPE_HIDDEN terminate_handler final {
    public:
      terminate_handler() : m_old_handler(std::set_terminate(&handle_terminate))
      { }

      ~terminate_handler() {
        std::set_terminate(m_old_handler);
      }
    private:
      std::terminate_handler m_old_handler;
    };
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<terminate_handler> terminate_handler_definition(
      "exceptions.terminate_handler",
      "Terminate handler",
      s::initialize_by_default = s::initialization_stage::before_static_configuration,
      s::global_singleton
    );
  }
}
