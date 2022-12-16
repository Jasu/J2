#include "logging/logging.hpp"

#include "mem/shared_ptr.hpp"

#include "ansi/sgr_styled_sink.hpp"
#include "files/paths/path.hpp"
#include "files/fds/open.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "files/fds/adopt.hpp"
#include "streams/fd_sink.hpp"
#include "exceptions/assert_lite.hpp"
#include "services/services.hpp"
#include "strings/string.hpp"
#include "strings/formatting/context.hpp"
#include "logging/global.hpp"

#include <termios.h>

namespace j::logging {
  namespace s = j::strings;
  namespace sa = s::attributes;

  logger_context::logger_context(logger_context * parent, strings::styled_string && title) noexcept
    : parent(parent),
      title(static_cast<strings::styled_string &&>(title))
  {
  }

  logger::logger() noexcept { }
  global_logger::global_logger() noexcept { }

  logger::logger(mem::shared_ptr<strings::formatted_sink> sink, strings::const_string_view channel) noexcept {
    initialize(static_cast<mem::shared_ptr<strings::formatted_sink> &&>(sink), channel);
  }

  void logger::initialize(mem::shared_ptr<strings::formatted_sink> sink, strings::const_string_view channel) {
    m_sink = static_cast<mem::shared_ptr<strings::formatted_sink> &&>(sink);
    m_channel = channel;
    J_ASSERT_NOT_NULL(m_channel, m_sink);
  }

  void logger::set_sink(mem::shared_ptr<streams::sink> sink) {
    J_ASSERT(sink);
    m_sink = mem::make_shared<strings::formatted_sink>(sink);
  }

  void logger::set_sink_to_file(const strings::const_string_view & path) {
    files::path p{path};
    auto fd = files::fds::open(p, files::open_flags::write | files::open_flags::create
      | files::open_flags::truncate);
    J_ASSERT(fd);
    set_sink(mem::make_shared<streams::fd_sink>(fd));
  }

  void logger::ensure_sink() {
    if (!m_sink) {
      m_sink = mem::make_shared<strings::formatted_sink>(
        mem::static_pointer_cast<strings::styled_sink>(
          mem::make_shared<ansi::sgr_styled_sink>(mem::make_shared<streams::fd_sink>(j::files::fds::adopt(2))))
      );
    }
    J_ASSERT_NOT_NULL(m_sink);
  }

  void J_HIDDEN logger::maybe_write_header(severity sev) {
    if (!s::formatting_context->is_multiline && sev <= severity::warning) {
      m_sink->write("{rainbow:<15+1} {} ", m_channel, sev);
    }
  }

  void logger::log(severity sev, bool newline, strings::const_string_view format_string) noexcept {
    do_log(sev, newline, format_string, 0U, nullptr);
  }

  inline static void print_title(strings::formatted_sink & sink, logger_context * J_NOT_NULL ctx) {
    if (!ctx->title) {
      return;
    }
    if (ctx->parent) {
      print_title(sink, ctx->parent);
    }
    sink.write("\n");
    sink.write(ctx->title);
    sink.write("\n");
    ctx->title.clear();
  }

  void logger::do_log(severity sev, bool newline, strings::const_string_view format_string, u32_t num_params,
                      const strings::formatting::format_value * params) noexcept {
    ensure_sink();
    if (m_context.has_value()) {
      print_title(*m_sink, m_context.maybe_get());
    }
    maybe_write_header(sev);
    if (strings::formatting_context->indent) {
      m_sink->write("{indent}", strings::formatting_context->indent);
    }
    m_sink->format(format_string, num_params, params);
    if (newline) {
      m_sink->write("\n");
      m_sink->flush();
    }
  }

  void global_logger::initialize(mem::shared_ptr<strings::formatted_sink> sink) {
    logger::initialize(static_cast<mem::shared_ptr<strings::formatted_sink> &&>(sink), "Global");
  }

  void logger::flush() noexcept {
    if (m_sink) {
      m_sink->flush();
      ::tcdrain(0);
    }
  }

  void logger::reset_column() noexcept {
    if (m_sink) {
      m_sink->reset_column();
    }
  }
  void logger::pad_to_column(i32_t col) noexcept {
    if (m_sink) {
      m_sink->pad_to_column(col);
    }
  }

  global_logger global_logger::instance{};

  namespace {
    namespace s = services;
    mem::shared_ptr<logger> create_logger(const s::injection_context * ic, mem::shared_ptr<strings::formatted_sink> sink) {
      return mem::make_shared<logger>(
        static_cast<mem::shared_ptr<strings::formatted_sink> &&>(sink),
        ic->parent->service_metadata->name);
    }

    J_A(ND, NODESTROY) s::service_definition<logger> logger_definition(
      "logging.logger",
      "Per-service logger",
      s::create = s::factory(&create_logger,
        s::arg::autowire,
        s::service("stderr")),
      s::one_per_service
    );

    J_A(ND, NODESTROY) s::service_definition<global_logger> global_logger_definition(
      "logging.global_logger",
      "Global logger",
      s::create = &global_logger::instance,
      s::initialize = s::initializer_call(&global_logger::initialize, s::service("stderr")),
      s::initialize_by_default = s::initialization_stage::after_static_configuration
    );
  }
}
