#include "lisp/assembly/debug_formatters.hpp"
#include "lisp/assembly/target.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "lisp/assembly/register_info.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::assembly {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    class J_TYPE_HIDDEN phys_reg_formatter final : public s::formatter_known_length<phys_reg> {
    public:
      void do_format(
        const s::const_string_view &,
        const phys_reg & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        if (!value) {
          target.write("none");
        }
        if (asm_context.has_value()) {
          if (auto n = asm_context->target->get_reg_info()->name_of(value)) {
            target.write(n);
            return;
          }
        }
        target.write(value.is_fp() ? "f" : "i");
        f::integer_formatter_v<u8_t>.do_format("", (u8_t)value.index, target, current_style);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const phys_reg & value
      ) const noexcept override {
        if (!value) {
          return 4U;
        }
        if (asm_context.has_value()) {
          if (auto n = asm_context->target->get_reg_info()->name_of(value)) {
            return ::j::strlen(n);
          }
        }
        return 1U + f::integer_formatter_v<u8_t>.do_get_length("", (u8_t)value.index);
      }
    };

    class J_TYPE_HIDDEN reg_formatter final : public s::formatter_known_length<reg> {
    public:
      void do_format(
        const s::const_string_view &,
        const reg & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        if (!value) {
          target.write("none");
        }
        if (asm_context.has_value()) {
          if (auto n = asm_context->target->get_reg_info()->name_of(value)) {
            target.write(n);
            return;
          }
        }
        target.write(value.is_fp() ? "f" : "i");
        f::integer_formatter_v<u8_t>.do_format("", (u8_t)value.index(), target, current_style);
        target.write(".");
        f::integer_formatter_v<u8_t>.do_format("", width_bits(value.width()), target, current_style);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const reg & value
      ) const noexcept override {
        if (!value) {
          return 4U;
        }
        if (asm_context.has_value()) {
          if (auto n = asm_context->target->get_reg_info()->name_of(value)) {
            return ::j::strlen(n);
          }
        }
        return 2U
          + f::integer_formatter_v<u8_t>.do_get_length("", (u8_t)value.index())
          + f::integer_formatter_v<u8_t>.do_get_length("", width_bits(value.width()));
      }
    };

    J_A(ND, NODESTROY) const phys_reg_formatter g_preg_fmt;
    J_A(ND, NODESTROY) const reg_formatter g_reg_fmt;
  }
  const s::formatter_known_length<phys_reg> & g_preg_formatter = g_preg_fmt;
  const s::formatter_known_length<reg> & g_reg_formatter = g_reg_fmt;
}
