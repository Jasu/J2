#pragma once

#include "logging/severity.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "strings/formatting/context.hpp"
#include "strings/formatting/format_value.hpp"
#include "strings/styling/styled_string.hpp"

namespace j::strings::inline formatting {
  class formatted_sink;
}
namespace j::streams {
  class sink;
}

J_DECLARE_EXTERN_SHARED_PTR(j::strings::formatted_sink);

namespace j::logging {
  namespace attributes {
    namespace a = j::attributes;

    inline constexpr a::attribute_definition title{
      a::value_type = type<strings::styled_string>,
      a::tag = type<struct title_tag>};
  }

  struct logger_context final {
    J_A(AI,ND) inline logger_context() noexcept = default;
    J_A(AI,ND) inline logger_context(logger_context &&) noexcept = default;
    J_A(AI,ND) inline logger_context & operator=(logger_context &&) noexcept = default;
    logger_context(logger_context * parent, strings::styled_string && title) noexcept;

    logger_context * parent = nullptr;
    strings::styled_string title;
  };

  struct logger_context_guard final {
    template<typename T>
    inline explicit logger_context_guard(T && v) noexcept
      : formatting_guard(static_cast<T &&>(v)) {
    }

    template<typename T, typename U>
    inline explicit logger_context_guard(T && v, U && u) noexcept
      : formatting_guard(static_cast<T &&>(v)),
        logger_guard(static_cast<U &&>(u))
    { }

    util::context_stack<strings::formatting_context_t, true>::guard formatting_guard;
    util::context_stack<logger_context, true>::guard logger_guard;
  };

  class logger {
  private:
  protected:
    logger() noexcept;

    void initialize(mem::shared_ptr<strings::formatted_sink> sink, strings::const_string_view channel);

  public:
    logger(mem::shared_ptr<strings::formatted_sink> sink, strings::const_string_view channel) noexcept;

    void flush() noexcept;
    void reset_column() noexcept;
    void pad_to_column(i32_t col) noexcept;
    void set_sink(mem::shared_ptr<streams::sink> sink);
    void set_sink_to_file(const strings::const_string_view & path);

    template<typename... Params>
    J_A(ND,AI,HIDDEN) inline void debug(bool newline, strings::const_string_view format_string, Params && ... params) noexcept {
      log(severity::debug, newline, format_string, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_A(ND,AI,HIDDEN) inline void info(bool newline, strings::const_string_view format_string, Params && ... params) noexcept {
      log(severity::info, newline, format_string, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_A(ND,AI,HIDDEN) inline void notice(bool newline, strings::const_string_view format_string, Params && ... params) noexcept {
      log(severity::notice, newline, format_string, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_A(ND,AI,HIDDEN) inline void warning(bool newline, strings::const_string_view format_string, Params && ... params) noexcept {
      log(severity::warning, newline, format_string, static_cast<Params &&>(params)...);
    }

    template<typename... Params>
    J_A(ND,AI,HIDDEN) inline void error(bool newline, strings::const_string_view format_string, Params && ... params) noexcept {
      log(severity::error, newline, format_string, static_cast<Params &&>(params)...);
    }

    logger(const logger &) = delete;

    void log(severity sev, bool newline, strings::const_string_view format_string) noexcept;

    template<typename Param>
    J_A(ND,NI) inline void log(severity sev, bool newline, strings::const_string_view format_string, Param && param) noexcept {
      strings::formatting::format_value value{static_cast<Param &&>(param)};
      do_log(sev, newline, format_string, 1U, &value);
    }

    template<typename Param, typename... Params>
    J_A(ND) inline void log(severity sev,
                            bool newline,
                            strings::const_string_view format_string,
                            Param && param, Params && ... params) noexcept {
      strings::formatting::format_value values[] = {
        strings::formatting::format_value(static_cast<Param &&>(param)),
        strings::formatting::format_value(static_cast<Params &&>(params))...
      };
      do_log(sev, newline, format_string, 1U + sizeof...(Params), values);
    }

    void do_log(severity sev,
                bool newline,
                strings::const_string_view format_string,
                u32_t num_params,
                const strings::formatting::format_value * params) noexcept;

    template<typename... Attrs>
    J_A(NODISC,AI,ND,HIDDEN) inline auto begin(Attrs && ... attrs) noexcept {
      if constexpr (j::attributes::has<Attrs...>(attributes::title)) {
        return logger_context_guard(strings::formatting_context.enter(static_cast<Attrs &&>(attrs)...),
                                    m_context.enter(attributes::title.get(static_cast<Attrs &&>(attrs)...)));
      } else {
        return logger_context_guard(strings::formatting_context.enter(static_cast<Attrs &&>(attrs)...));
      }
    }

    void ensure_sink();
    void J_HIDDEN maybe_write_header(severity sev);

    mem::shared_ptr<strings::formatted_sink> m_sink;
    strings::string m_channel;
    util::context_stack<logger_context, true> m_context;
  };
}
