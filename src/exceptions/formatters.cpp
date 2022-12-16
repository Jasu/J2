#include "exceptions/formatters.hpp"

#include "exceptions/backtrace.hpp"
#include "exceptions/exceptions.hpp"
#include "colors/default_colors.hpp"
#include "hzd/demangle.hpp"
#include "mem/shared_ptr.hpp"
#include "services/service_definition.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "strings/string.hpp"
#include "strings/styling/styled_string.hpp"
#include "tags/formatter.hpp"
#include "type_id/type_id.hpp"

#include <typeinfo>

namespace j::exceptions {
  namespace {
    const strings::styling::style g_msg_style{ colors::colors::bright_yellow };
  }
  exception_formatter::exception_formatter(mem::shared_ptr<tags::tag_formatter> tag_formatter) noexcept
    : m_tag_formatter(static_cast<mem::shared_ptr<tags::tag_formatter> &&>(tag_formatter))
  {
    J_ASSERT_NOT_NULL(m_tag_formatter);
  }

  void exception_formatter::format(strings::formatted_sink & sink, exception & e) const {

    if (auto j_exception = dynamic_cast<exception *>(&e)) {
      const strings::string * service = j_exception->maybe_get_tag_value(service_name);
      const char * const * function = j_exception->maybe_get_tag_value(function_name);

      sink.write((service || function)
                 ? "{#error} Exception {#white}{}{/} {/}"
                 : "{#error} Exception {#white}{}{/}: {/}\n",
                 type_id::type_id(typeid(e)));
      if (service) {
        sink.write(function ? " in {#bright_green,bold}{}{/}" : " in {#bright_green,bold}{}{/}:\n", *service);
      }
      if (function) {
        sink.write(" when calling {#bright_yellow,bold}{}{/}:\n", *function);
      }

      if (auto msg = j_exception->maybe_get_tag_value(message)) {
        auto param = j_exception->maybe_get_tag(value);
        if (param) {
          vector<strings::formatting::format_value> values;
          while (param != j_exception->end()) {
            if (param->is(value)) {
              values.emplace_back(param->value().as_void_star(), param->value().type());
            }
            ++param;
          }
          sink.format(strings::const_string_view(*msg), values.size(), values.begin(), g_msg_style);
          sink.write("\n");
        } else {
          sink.write(g_msg_style, "  {}\n", *msg);
        }
      }

      if (auto desc = j_exception->maybe_get_tag_value(description)) {
        sink.write("  {#white}{}{/}\n", *desc);
      }
      sink.flush();
      // TODO print as table

      bool has_bt = false;
      if (auto bt = j_exception->maybe_get_tag_value(backtrace)) {
        sink.write("  {#red_bg,white,bold} Backtrace: {/}\n\n");
        for (auto & frame : *bt) {
          sink.write("  {#bright_green,bold}{:2}{/} ", frame.index);
          if (J_UNLIKELY(frame.type == j::backtrace::frame_type::signal)) {
            sink.write("  {#bright_red,bold} Signal {/}");
          } else if (J_UNLIKELY(frame.type == j::backtrace::frame_type::unknown)) {
            sink.write("  {#bright_red,bold} Unknown {/}");
          }
          if (J_UNLIKELY(!frame.func_name)) {
            sink.write("{#bright_red,bold}<Unknown function>{/}\n");
          } else {
            try {
              strings::string func_name{frame.func_name + strings::string(1, '\0')};
              const char * demangled = demangle(func_name.data());
              sink.write("{#bright_magenta,bold}{}{/}\n", demangled);
              demangle_free(demangled);
            } catch (...) {
              sink.write("{#bright_yellow}{}{/}\n", frame.func_name);
            }
          }
          has_bt = true;
        }
      }
      sink.flush();

      for (auto tag = j_exception->begin(), end = j_exception->end(); tag != end; ++tag) {
        auto def = &tag->definition();
        if (def == &message || def == &backtrace || def == &value || def == &function_name ||
            def == &description || def == &service_name) {
          continue;
        }
        if (has_bt) {
          sink.write("\n");
          has_bt = false;
        }
        switch (def->name_format) {
        case tags::tag_name_format::unformatted: sink.write("{#bright_cyan}{}{/}", def->name); break;
        case tags::tag_name_format::formatted: sink.write(def->name); break;
        case tags::tag_name_format::with_index: {
          u32_t index = 0U;
          for (auto t2 = j_exception->begin(); t2 != tag; ++t2) {
            if (&t2->definition() == def) {
              ++index;
            }
          }
          sink.write(def->name, index);
          break;
        }
        }
        sink.write(": {}\n", m_tag_formatter->format_styled(*tag));
      }
      sink.flush();

      return;
    }
    // TODO
    sink.write("{#error} Exception {/} {#light_gray}No formatter available.{/}");
    return;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<exception_formatter> exception_formatter_definition(
      "exceptions.formatter",
      "Exception formatter",
      s::create = s::constructor<mem::shared_ptr<tags::tag_formatter>>(),
      s::initialize_by_default = s::initialization_stage::before_static_configuration
    );
  }
}
