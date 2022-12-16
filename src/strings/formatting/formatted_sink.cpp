#include "strings/formatting/formatted_sink.hpp"

#include "services/service_definition.hpp"
#include "services/service_instance.hpp"
#include "streams/fd_sink.hpp"
#include "strings/attributes.hpp"
#include "strings/styling/dummy_styled_sink.hpp"

namespace j::strings {
  inline namespace formatting {
    formatted_sink::formatted_sink(const mem::shared_ptr<styling::styled_sink> & sink) noexcept
      : sink(sink)
    {
      J_ASSERT_NOT_NULL(this->sink);
    }

    formatted_sink::formatted_sink(const mem::shared_ptr<streams::sink> & sink) noexcept
      : sink(mem::make_shared<styling::dummy_styled_sink>(sink))
    {
      J_ASSERT_NOT_NULL(this->sink);
    }

    u32_t formatted_sink::write_unformatted(const_string_view s) noexcept {
      return sink->write(s);
    }

    u32_t formatted_sink::write_unformatted(const char * J_NOT_NULL s) noexcept {
      return sink->write(s);
    }

    u32_t formatted_sink::write_unformatted(styling::style style, const_string_view s) noexcept {
      return sink->write_styled(style, s);
    }

    void formatted_sink::flush() noexcept {
      sink->flush();
    }
    void formatted_sink::reset_column() noexcept {
      sink->reset_column();
    }
    void formatted_sink::pad_to_column(i32_t col) noexcept {
      sink->pad_to_column(col);
    }
    u32_t formatted_sink::write_styled(const style & style, const strings::const_string_view & str) noexcept {
      return sink->write_styled(style, str);
    }

    u32_t formatted_sink::write_styled(const style & style, const char * J_NOT_NULL str) noexcept {
      return sink->write_styled(style, str);
    }

    formatted_sink::~formatted_sink() {
      if (sink) {
        sink->flush();
      }
    }

    namespace {
      namespace s = services;
      J_A(ND, NODESTROY) s::service_definition<formatted_sink> def(
        "strings.formatted_sink",
        "Formatted Sink",
        s::no_default_instance,
        s::create = s::constructor<mem::shared_ptr<styling::styled_sink>>(
          s::arg::service_attribute(attributes::sink)));

      J_A(ND, NODESTROY) s::service_instance<formatted_sink> stdout_def(
        "stdout",
        "Formatted Standard Output",
        attributes::sink = s::service("files.styled_stdout"));

      J_A(ND, NODESTROY) s::service_instance<formatted_sink> stderr_def(
        "stderr",
        "Formatted Standard Error",
        attributes::sink = s::service("files.styled_stderr"));

    }
  }
}

J_DEFINE_EXTERN_SHARED_PTR(j::strings::formatted_sink);
