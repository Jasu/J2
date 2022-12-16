#include "events/event_loop.hpp"
#include "events/poll_handle.hpp"
#include "events/signal_handle.hpp"
#include "events/prepare_handle.hpp"
#include "events/stream_handle.hpp"
#include "colors/default_colors.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/default_styles.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    namespace s = strings;
    namespace c = colors::colors;
    namespace f = s::formatters;

    const s::style loop_bg{c::white, c::magenta, s::bold};
    const s::style running_bg{c::bright_green, c::bright_yellow, s::bold};
    const s::style not_running_bg{c::gray, c::white, s::bold};

    static const f::debug_enum_entry handle_type_entries[] = {
      [(u8_t)handle_type::none] = { " None     ", s::styles::error },
      [(u8_t)handle_type::poll] = { " Poll     ", s::styles::tag1 },
      [(u8_t)handle_type::tty]  = { " TTY      ", s::styles::tag2 },
      [(u8_t)handle_type::signal]  = { " Signal   ", s::styles::tag3 },
      [(u8_t)handle_type::prepare]  = { " Prepare   ", s::styles::tag4 },
    };

    static const f::debug_enum_entry event_type_entries[] = {
      [(u8_t)event_type::error]        = { "Error     ", s::styles::bright_red },
      [(u8_t)event_type::poll]         = { "Poll      ", s::styles::bright_cyan },
      [(u8_t)event_type::after_close]  = { "PostClose ", s::styles::bright_red },
      [(u8_t)event_type::before_close] = { "PreClose  ", s::styles::bright_red },
      [(u8_t)event_type::write]        = { "Write     ", s::styles::bright_yellow },
      [(u8_t)event_type::signal]       = { "Signal    ", s::styles::bright_magenta },
      [(u8_t)event_type::prepare]       = { "Prepare    ", s::styles::light_gray },
      [(u8_t)event_type::read]         = { "Read      ", s::styles::bright_green },
    };

    static const f::debug_enum_entry tty_mode_entries[] = {
      [(u8_t)tty_mode::none]   = { " None ", s::styles::tag2 },
      [(u8_t)tty_mode::normal] = { " Norm ", s::styles::tag3 },
      [(u8_t)tty_mode::raw]    = { " Raw  ", s::styles::tag4 },
    };

    static const f::debug_enum_entry eof_mode_entries[] = {
      [(u8_t)eof_mode::close]        = { "Close", s::styles::light_gray },
      [(u8_t)eof_mode::stop_reading] = { "Stop ", s::styles::bright_red },
    };

    static const f::debug_enum_entry handle_state_entries[] = {
      [(u8_t)handle_state::ready]   = { "Ready  ", s::styles::light_gray },
      [(u8_t)handle_state::running] = { "Running", s::styles::bright_green },
      [(u8_t)handle_state::closing] = { "Closing", s::styles::bright_red },
      [(u8_t)handle_state::closed]  = { "Closed ", s::styles::bright_red },
    };

    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<handle_type> handle_type_formatter(handle_type_entries);
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<event_type> event_type_formatter(event_type_entries);
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<tty_mode> tty_mode_formatter(tty_mode_entries);
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<eof_mode> eof_mode_formatter(eof_mode_entries);
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<handle_state> handle_state_formatter(handle_state_entries);


    struct J_TYPE_HIDDEN poll_flags_formatter final : public s::formatter_known_length<poll_flags> {
      void do_format(
        const s::const_string_view &,
        const poll_flags & v,
        s::styled_sink & to,
        s::style) const override {
        to.write_styled(v.has(poll_flag::readable) ? s::styles::bright_green.with_bold() : s::styles::light_gray,
                        v.has(poll_flag::readable) ? "R" : "-");
        to.write_styled(v.has(poll_flag::writable) ? s::styles::bright_red.with_bold() : s::styles::light_gray,
                        v.has(poll_flag::writable) ? "W" : "-");
      }

      u32_t do_get_length(const s::const_string_view &, const poll_flags &) const noexcept override {
        return 2U;
      }
    };

    J_A(ND, NODESTROY) const poll_flags_formatter poll_flags_fmt;

    struct J_TYPE_HIDDEN handler_record_formatter final : public s::formatter_known_length<detail::event_handler_record> {
      void do_format(
        const s::const_string_view & type,
        const detail::event_handler_record & v,
        s::styled_sink & to,
        s::style cs) const override
      {
        to.write("      ");
        if (!v.mask) {
          to.write_styled(s::styles::error, " NO MASK ");
        } else if (type == "common") {
          u32_t bits = bits::ctz_safe(v.mask);
          event_type_formatter.do_format("", (event_type)bits, to, cs);
        } else if (type == "poll") {
          poll_flags_fmt.do_format("", poll_flags(v.mask), to, cs);
        } else if (type == "read") {
          if (v.mask & 1) {
            to.write_styled(s::styles::bright_green, "Read");
          }
          if (v.mask & 2) {
            to.write_styled(s::styles::bright_yellow, "Eof");
          }
        } else {
          if (v.mask == U16_MAX) {
            to.write_styled(s::styles::bright_green, "Any event");
          } else {
            to.write_styled(s::styles::bright_yellow, "Mask: ");
            f::integer_formatter_v<u16_t>.do_format("08B", v.mask, to, cs);
          }
        }
        if (v.is_marked_for_deletion) {
          to.write_styled(s::styles::bright_red, " DEL");
        }
        if (v.is_one_shot) {
          to.write_styled(s::styles::bright_yellow, " ONE-SHOT");
        }
        to.write(" \n");
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & type,
        const detail::event_handler_record & v
      ) const noexcept override {
        u32_t result = 6U;
        if (!v.mask) {
          result += 9U;
        } else if (type == "common") {
          u32_t bits = bits::ctz_safe(v.mask);
          result += event_type_formatter.do_get_length("", (event_type)bits);
        } else if (type == "poll") {
          result += 2U;
        } else if (type == "read") {
          if (v.mask & 1) {
            result += 4U;
          }
          if (v.mask & 2) {
            result += 3U;
          }
        } else {
          if (v.mask == U16_MAX) {
            result += 9U;
          } else {
            result += 6U + 8U;
          }
        }
        if (v.is_marked_for_deletion) {
          result += 4U;
        }
        if (v.is_one_shot) {
          result += 9U;
        }
        return result + 2U;
      }
    };

    J_A(ND, NODESTROY) const handler_record_formatter handler_rec_fmt;

    template<typename T>
    struct J_TYPE_HIDDEN handle_formatter final : public s::formatter_known_length<T> {
      void do_format(
        const s::const_string_view &,
        const T & v,
        s::styled_sink & to,
        s::style cs) const override {

        to.write("    ");
        handle_type_formatter.do_format("", v.type, to, cs);
        handle_state_formatter.do_format("", v.state, to, cs);
        to.write(" FD=");
        f::integer_formatter_v<i32_t>.do_format("", v.fd, to, cs);
        to.write("\n");

        // if constexpr(is_same_v<T, stream_handle>) {
        // }

        to.write("    ");
        to.write_styled(s::styles::tag1, " Common events: ");
        to.write("\n");
        for (auto & h : v.common_events.handlers) {
          handler_rec_fmt.do_format("common", h, to, cs);
        }

        if constexpr (type_list_contains_v<typename T::events_t, poll_event>) {
          to.write("    ");
          to.write_styled(s::styles::tag2, " Poll events: ");
          to.write("\n");
          for (auto & h : v.template handlers<poll_event>()) {
            handler_rec_fmt.do_format("poll", h, to, cs);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, write_event>) {
          to.write("    ");
          to.write_styled(s::styles::tag3, " Write events: ");
          to.write("\n");
          for (auto & h : v.template handlers<write_event>()) {
            handler_rec_fmt.do_format("write", h, to, cs);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, read_event>) {
          to.write("    ");
          to.write_styled(s::styles::tag4, " Read events: ");
          to.write("\n");
          for (auto & h : v.template handlers<read_event>()) {
            handler_rec_fmt.do_format("read", h, to, cs);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, signal_event>) {
          to.write("    ");
          to.write_styled(s::styles::tag4, " Signal events: ");
          to.write("\n");
          for (auto & h : v.template handlers<signal_event>()) {
            handler_rec_fmt.do_format("signal", h, to, cs);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, prepare_event>) {
          to.write("    ");
          to.write_styled(s::styles::tag5, " Prepare events: ");
          to.write("\n");
          for (auto & h : v.template handlers<prepare_event>()) {
            handler_rec_fmt.do_format("prepare", h, to, cs);
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const T & v
      ) const noexcept override {
        u32_t result = 4U + 10U + 11U + 4U + f::integer_formatter_v<i32_t>.do_get_length("", v.fd) + 3U;

        result += 4U + 17U;
        for (auto & h : v.common_events.handlers) {
          result += 1U + handler_rec_fmt.do_get_length("common", h);
        }

        if constexpr (type_list_contains_v<typename T::events_t, poll_event>) {
          result += 4U + 15U;
          for (auto & h : v.template handlers<poll_event>()) {
            result += 1U + handler_rec_fmt.do_get_length("poll", h);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, write_event>) {
          result += 4U + 16U;
          for (auto & h : v.template handlers<write_event>()) {
            result += 1U + handler_rec_fmt.do_get_length("write", h);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, read_event>) {
          result += 4U + 15U;
          for (auto & h : v.template handlers<read_event>()) {
            result += 1U + handler_rec_fmt.do_get_length("read", h);
          }
        }

        if constexpr (type_list_contains_v<typename T::events_t, signal_event>) {
          result += 4U + 17U;
          for (auto & h : v.template handlers<signal_event>()) {
            result += 1U + handler_rec_fmt.do_get_length("signal", h);
          }
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const handle_formatter<poll_handle> poll_handle_fmt;
    J_A(ND, NODESTROY) const handle_formatter<stream_handle> stream_handle_fmt;
    J_A(ND, NODESTROY) const handle_formatter<signal_handle> signal_handle_fmt;
    J_A(ND, NODESTROY) const handle_formatter<prepare_handle> prepare_handle_fmt;

    struct J_TYPE_HIDDEN handle_visitor final {
      s::styled_sink & to;
      s::style cs;

      void format(uv_handle_t * handle) {
        to.write("  ");
        to.write_styled(s::styles::tag3, " ");
        to.write_styled(s::styles::tag3, uv_handle_type_name(handle->type));
        to.write_styled(s::styles::tag3, " ");
        if (uv_has_ref(handle)) {
          to.write_styled(s::styles::light_gray, " Ref  ");
        } else {
          to.write_styled(s::styles::bright_red,   " NoRef");
        }
        if (uv_is_active(handle)) {
          to.write_styled(s::styles::bright_green, " Active  ");
        } else {
          to.write_styled(s::styles::bright_red,   " Inactive");
        }
        if (uv_is_closing(handle)) {
          to.write_styled(s::styles::bright_red, " Closing");
        }
        if (handle->data) {
          auto bh = reinterpret_cast<base_handle*>(handle->data);
          if (bh->type == handle_type::tty) {
            stream_handle_fmt.do_format("", *bh->as_stream_handle(), to, cs);
          } else if (bh->type == handle_type::poll) {
            poll_handle_fmt.do_format("", *bh->as_poll_handle(), to, cs);
          } else if (bh->type == handle_type::signal) {
            signal_handle_fmt.do_format("", *bh->as_signal_handle(), to, cs);
          } else if (bh->type == handle_type::prepare) {
            prepare_handle_fmt.do_format("", *bh->as_prepare_handle(), to, cs);
          } else {
            to.write("    ");
            to.write_styled(s::styles::error, " Unknown type ");
          }
          to.write("\n");
        }
        to.write("\n");
      }

      static void visit(uv_handle_t * handle, void * arg) {
        ((handle_visitor*)(arg))->format(handle);
      }
    };

    struct J_TYPE_HIDDEN handle_size_visitor final {
      u32_t result = 0U;
      void get_size(uv_handle_t * handle) {
        result += 3U + strlen(uv_handle_type_name(handle->type)) + 1U + 6U + 9U;
        if (uv_is_closing(handle)) {
          result += 8U;
        }
        ++result;
        if (handle->data) {
          auto bh = reinterpret_cast<base_handle*>(handle->data);
          if (bh->type == handle_type::tty) {
            result += stream_handle_fmt.do_get_length("", *bh->as_stream_handle());
          } else if (bh->type == handle_type::poll) {
            result += poll_handle_fmt.do_get_length("", *bh->as_poll_handle());
          } else if (bh->type == handle_type::signal) {
            result += signal_handle_fmt.do_get_length("", *bh->as_signal_handle());
          } else if (bh->type == handle_type::prepare) {
            result += prepare_handle_fmt.do_get_length("", *bh->as_prepare_handle());
          } else {
            result += 2U + 14U;
          }
          ++result;
        }
        ++result;
      }

      static void visit(uv_handle_t * handle, void * arg) {
        ((handle_size_visitor*)(arg))->get_size(handle);
      }
    };

    struct J_TYPE_HIDDEN event_loop_formatter final : public s::formatter_known_length<event_loop> {
      void do_format(
        const s::const_string_view &,
        const event_loop & v,
        s::styled_sink & to,
        s::style cs) const override
      {
        uv_loop_t * loop = const_cast<event_loop&>(v).uv_loop();
        to.write_styled(loop_bg, " Event loop ");
        to.write_styled(v.is_running ? running_bg : not_running_bg,
                        v.is_running ? " Running     " : " Not running ");
        to.write(" ");
        to.write_styled(uv_loop_alive(loop) ? running_bg : not_running_bg,
                        uv_loop_alive(loop) ? " Alive     " : " Not alive ");
        to.write("\n");

        handle_visitor visitor{to, cs};
        uv_walk(loop, &handle_visitor::visit, (void*)&visitor);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const event_loop & v
      ) const noexcept override {
        uv_loop_t * loop = const_cast<event_loop&>(v).uv_loop();
        u32_t result = 12U + 13U + 1U + 11U + 1U;
        handle_size_visitor visitor;
        uv_walk(loop, &handle_visitor::visit, (void*)&visitor);
        return result + visitor.result;
      }
    };
    J_A(ND, NODESTROY) const event_loop_formatter event_loop_fmt;
  }
}
