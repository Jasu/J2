#include "lisp/common/debug_formatters.hpp"
#include "lisp/assembly/debug_formatters.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "lisp/assembly/amd64/operand_mask.hpp"

namespace j::lisp::assembly::amd64 {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    J_A(ND, NODESTROY) const f::enum_formatter<operand_type_mask> g_operand_mask_formatter{{
        {operand_type_mask::r8,      "r8",     s::styles::bright_cyan},
        {operand_type_mask::m8,      "m8",     s::styles::bright_yellow},
        {operand_type_mask::rm8,     "rm8",    s::styles::bright_green},
        {operand_type_mask::imm8,    "imm8",   s::styles::bright_magenta},

        {operand_type_mask::r16,     "r16",    s::styles::bright_cyan},
        {operand_type_mask::m16,     "m16",    s::styles::bright_yellow},
        {operand_type_mask::rm16,    "rm16",   s::styles::bright_green},
        {operand_type_mask::imm16,   "imm16",  s::styles::bright_magenta},

        {operand_type_mask::r32,     "r32",    s::styles::bright_cyan},
        {operand_type_mask::m32,     "m32",    s::styles::bright_yellow},
        {operand_type_mask::rm32,    "rm32",   s::styles::bright_green},
        {operand_type_mask::imm32,   "imm32",  s::styles::bright_magenta},

        {operand_type_mask::r64,     "r64",    s::styles::bright_cyan},
        {operand_type_mask::m64,     "m64",    s::styles::bright_yellow},
        {operand_type_mask::rm64,    "rm64",   s::styles::bright_green},
        {operand_type_mask::imm64,   "imm64",  s::styles::bright_magenta},

        {operand_type_mask::mem,     "m",      s::styles::bright_yellow},
        {operand_type_mask::imm,     "imm",    s::styles::bright_magenta},
        {operand_type_mask::any_imm,"any_imm", s::styles::bright_magenta},

        {operand_type_mask::xmm,     "xmm",    s::styles::bright_red},
        {operand_type_mask::ymm,     "ymm",    s::styles::bright_red},

        {operand_type_mask::xmm_m64,"xmm/m64", s::styles::bright_red},
        {operand_type_mask::xmm_m32,"xmm/m32", s::styles::bright_red},

        {operand_type_mask::none,    "none",   s::styles::gray},
      }};

    class J_TYPE_HIDDEN operand_types_formatter final : public s::formatter_known_length<u32_t> {
    public:
      operand_types_formatter() noexcept
        : formatter_known_length<u32_t>("optypes", false)
      {
      }

      void do_format(
        const s::const_string_view & fo,
        const u32_t & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        bool is_first = true;
        for (u32_t i = 0U; i < 3U; ++i) {
          if (!is_first) {
            target.write(", ");
          }

          auto m = (operand_type_mask)(u8_t)(value >> ((2 - i) * 8));
          if (!is_first || m != operand_type_mask::none) {
            is_first = false;
            g_operand_mask_formatter.do_format(fo, m, target, current_style);
          }
        }

        if (is_first) {
          g_operand_mask_formatter.do_format(fo, operand_type_mask::none, target, current_style);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & fo,
        const u32_t & value
      ) const noexcept override {
        u32_t sz = 0U;
        bool is_first = true;
        for (u32_t i = 0U; i < 3U; ++i) {
          if (!is_first) {
            sz += 2U;
          }

          auto m = (operand_type_mask)(u8_t)(value >> ((2 - i) * 8));
          if (!is_first || m != operand_type_mask::none) {
            is_first = false;
            sz += g_operand_mask_formatter.do_get_length(fo, m);
          }
        }
        return sz ? sz : 4U;
      }
    };

    J_A(ND, NODESTROY) const operand_types_formatter g_operand_types_formatter;

    class J_TYPE_HIDDEN operand_formatter final : public s::formatter_known_length<operand> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const operand & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        switch (value.type()) {
        case operand_type::none:
          target.write("none");
          break;
        case operand_type::reg:
          g_reg_formatter.do_format(fo, value.reg(), target, current_style);
          break;
        case operand_type::imm64:
          f::integer_formatter_v<i64_t>.do_format("", value.imm64(), target, current_style);
          break;
        case operand_type::imm32:
          f::integer_formatter_v<i64_t>.do_format("", value.imm32(), target, current_style);
          break;
        case operand_type::imm16:
          f::integer_formatter_v<i64_t>.do_format("", value.imm16(), target, current_style);
          break;
        case operand_type::imm8:
          f::integer_formatter_v<i64_t>.do_format("", value.imm8(), target, current_style);
          break;
        case operand_type::rel8:
        case operand_type::rel16:
        case operand_type::rel32:
        case operand_type::rel64:
          switch (value.type()) {
          case operand_type::rel64: target.write("QWORD"); break;
          case operand_type::rel32: target.write("DWORD"); break;
          case operand_type::rel16: target.write("WORD"); break;
          case operand_type::rel8: target.write("BYTE"); break;
          default: J_UNREACHABLE();
          }
          target.write(" [rip + ");
          f::integer_formatter_v<i64_t>.do_format("", value.disp(), target, current_style);
          target.write("]");
          break;
        case operand_type::mem64:
        case operand_type::mem32:
        case operand_type::mem16:
        case operand_type::mem8:
          switch (value.type()) {
          case operand_type::mem64:
            target.write("QWORD");
            break;
          case operand_type::mem32:
            target.write("DWORD");
            break;
          case operand_type::mem16:
            target.write("WORD");
            break;
          case operand_type::mem8:
            target.write("BYTE");
            break;
          default: J_UNREACHABLE();
          }

          target.write(" [");
          g_reg_formatter.do_format(fo, value.base(), target, current_style);
          if (value.index()) {
            target.write(" + ");
            g_reg_formatter.do_format(fo, value.index(), target, current_style);
            target.write(" ");
            g_mem_scale_formatter.do_format("", value.scale(), target, current_style);
          }
          if (value.disp()) {
            target.write(" + ");
            f::integer_formatter_v<i64_t>.do_format("", value.disp(), target, current_style);
          }
          target.write("]");
          break;
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & fo,
        const operand & value
      ) const noexcept override {
        u32_t res = 0;
        switch (value.type()) {
        case operand_type::none:
          return 4U;
        case operand_type::reg:
          return g_reg_formatter.do_get_length(fo, value.reg());
        case operand_type::imm8:
          return f::integer_formatter_v<i64_t>.do_get_length("", value.imm8());
        case operand_type::imm16:
          return f::integer_formatter_v<i64_t>.do_get_length("", value.imm16());
        case operand_type::imm32:
          return f::integer_formatter_v<i64_t>.do_get_length("", value.imm32());
        case operand_type::imm64:
          return f::integer_formatter_v<i64_t>.do_get_length("", value.imm64());
        case operand_type::rel64:
        case operand_type::rel32:
          ++res; // DWORD/QWORD
          [[fallthrough]];
        case operand_type::rel16:
        case operand_type::rel8:
          res += 5U; // BYTE/WORD + SPC
          res += 7U; // "[rip + "
          res += f::integer_formatter_v<i64_t>.do_get_length("", value.disp());
          res += 1U; // "]"
          return res;
        case operand_type::mem64:
        case operand_type::mem32:
          res += 1U;
          [[fallthrough]];
        case operand_type::mem16:
        case operand_type::mem8: {
          res += 6U;
          res += g_reg_formatter.do_get_length(fo, value.base());
          if (value.index()) {
            res += 3U + g_reg_formatter.do_get_length(fo, value.index()) + 1U
              + g_mem_scale_formatter.do_get_length("", value.scale());
          }
          if (value.disp()) {
            res += 3U;
            res += f::integer_formatter_v<i64_t>.do_get_length("", value.disp());
          }
          res += 1U;
          return res;
        }
        }
      }
    };
    J_A(ND, NODESTROY) const operand_formatter g_operand_formatter;
  }
}
