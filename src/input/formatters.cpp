#include "strings/formatting/formatter.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/default_styles.hpp"
#include "input/key_event.hpp"

#include <xkbcommon/xkbcommon.h>

namespace j::input {
  namespace {
    namespace s = j::strings;
    J_A(ND, NODESTROY) const s::formatters::enum_formatter<modifier> modifier_formatter{{
        {modifier::shift,     "Shift",    s::styles::bright_green},
        {modifier::ctrl,      "Ctrl",     s::styles::bright_cyan},
        {modifier::alt,       "Alt",      s::styles::bright_magenta},
        {modifier::super,     "Super",    s::styles::bright_yellow},
        {modifier::alt_gr,    "AltGr",    s::styles::red},
        {modifier::caps_lock, "CapsLock", s::styles::light_gray},
      }};

    const char * const g_empty_set = "∅";

    class J_TYPE_HIDDEN modifier_mask_formatter final : public s::formatter_known_length<modifier_mask> {
    public:
      void do_format(
        const s::const_string_view &,
        const modifier_mask & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        if (value.empty()) {
          target.write_styled(s::styles::gray, g_empty_set, ::j::strlen(g_empty_set));
          return;
        }
        bool is_first = true;
        for (auto mod : g_modifiers) {
          if (!value.has(mod)) {
            continue;
          }
          if (!is_first) {
            target.write("+", 1);
          }
          target.set_style(s::styles::bright_yellow);
          modifier_formatter.do_format("", mod, target, s::styles::bright_yellow);
          target.set_style(init_style);
          is_first = false;
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const modifier_mask & value
      ) const noexcept override {
        if (value.empty()) {
          return ::j::strlen(g_empty_set);
        }
        u32_t len = 0U;
        bool is_first = true;
        for (auto mod : g_modifiers) {
          if (!value.has(mod)) {
            continue;
          }
          if (!is_first) {
            ++len;
          }
          len += modifier_formatter.do_get_length("", mod);
          is_first = false;
        }
        return len;
      }
    };

    J_A(ND, NODESTROY) const modifier_mask_formatter g_modifier_mask_formatter;

    class J_TYPE_HIDDEN key_event_formatter final : public s::formatter_known_length<key_event> {
    public:
      void do_format(
        const s::const_string_view &,
        const key_event & value,
        s::styled_sink & target,
        s::style init_style
      ) const override {
        if (value.is_repeat) {
          target.write_styled(s::styles::bright_red, "Repeat ", ::j::strlen("Repeat "));
        }
        s::formatters::integer_formatter_v<u32_t>.do_format("", value.keycode, target, init_style);
        if (value.modifiers) {
          target.write(" ", 1);
          target.set_style(s::styles::bright_yellow);
          g_modifier_mask_formatter.do_format("", value.modifiers, target, s::styles::bright_yellow);
          target.set_style(init_style);
        }
        target.write(" [", 2);

        bool is_first = true;
        for (u32_t sym : value.keysyms) {
          if (!is_first) {
            target.write(", ", 2);
          }
          s::formatters::integer_formatter_v<u32_t>.do_format("", sym, target, init_style);
          is_first = false;
        }
        target.write("] \"", 3);
        target.write(value.utf8);
        target.write("\"", 1);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const key_event & value
      ) const noexcept override {
        const u32_t mod_length = g_modifier_mask_formatter.do_get_length("", value.modifiers);
        u32_t syms_length = 0;
        for (u32_t sym : value.keysyms) {
          syms_length += s::formatters::integer_formatter_v<u32_t>.do_get_length("", sym);
          if (syms_length) {
            syms_length += 2U;
          }
        }
        return
          (value.is_repeat ? ::j::strlen("Repeat ") : 0U)
          + s::formatters::integer_formatter_v<u32_t>.do_get_length("", value.keycode)
          + 1
          + (mod_length ? mod_length + 1U : 0)
          + 2 + syms_length
          + 1
          + 2 + value.utf8.size();
      }
    };

    J_A(ND, NODESTROY) const key_event_formatter g_key_event_formatter;

    struct J_TYPE_HIDDEN keysym_formatter final : public s::formatter_known_length<u32_t> {
      keysym_formatter() noexcept
        : formatter_known_length<u32_t>("xkb-keysym", false)
      { }

      void do_format(
        const s::const_string_view &,
        const u32_t & v,
        s::styled_sink & to,
        s::style
      ) const override {
        char buffer[64];
        i32_t len = xkb_keysym_get_name(v, buffer, 64);
        if (len < 0) {
          to.write_styled(s::styles::error, "Unknown");
        }
        to.write(buffer, len);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const u32_t & v
      ) const noexcept override {
        char buffer[64];
        i32_t len = xkb_keysym_get_name(v, buffer, 64);
        return len < 0 ? 7U : len;
      }
    };
    J_A(ND, NODESTROY) const keysym_formatter g_keysym_formatter;
  }
}
