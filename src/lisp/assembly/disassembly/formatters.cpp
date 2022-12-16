#include "colors/default_colors.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/formatters/indent.hpp"
#include "lisp/values/type_formatters.hpp"
#include "lisp/sources/debug_formatters.hpp"
#include "strings/styling/styled_sink.hpp"
#include "lisp/assembly/disassembly/instruction.hpp"
#include "lisp/env/debug_formatters.hpp"

namespace j::lisp::assembly::disassembly {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;
    using color = colors::color;
    namespace c = colors::colors;

    const s::style label{c::bright_cyan, s::bold};

    const s::style invalid{c::bright_red, s::bold};

    const s::style mnemonic_default{c::white, s::bold};

    const s::style mnemonics[12] = {
      mnemonic_default,
      s::style{c::yellow}, // Stack
      s::style{c::cyan}, // Mov
      s::style{c::bright_green, s::bold}, // Read
      s::style{c::yellow, s::bold}, // Write
      s::style{c::bright_yellow}, // Cmp
      s::style{c::bright_blue}, // Bitwise
      s::style{c::bright_cyan}, // Integer
      s::style{c::yellow, s::bold}, // Jmp
      s::style{c::bright_yellow, s::bold}, // Conditional jmp
      s::style{c::bright_magenta}, // Floating point
      s::style{c::bright_red, s::bold}, // Error
    };

    const s::style comment{c::light_gray, s::bold};
    const s::style value_register{c::bright_green, s::bold};
    const s::style value_immediate{c::bright_magenta, s::bold};
    const s::style value_address{c::bright_yellow, s::bold};
    const s::style value_brackets{c::light_gray};

    const s::style scale{c::bright_magenta};
    const s::style displacement{c::bright_magenta};

    const strings::const_string_view brackets[]{
      [(u8_t)mem_width::none] = "[",
      [(u8_t)mem_width::byte] = "B[",
      [(u8_t)mem_width::word] = "W[",
      [(u8_t)mem_width::dword] = "DW[",
      [(u8_t)mem_width::qword] = "QW[",

      [(u8_t)mem_width::vec128] = "V128[",
      [(u8_t)mem_width::vec256] = "V256[",
      [(u8_t)mem_width::vec512] = "V512[",
    };

    class J_TYPE_HIDDEN value_formatter final : public s::formatter_known_length<value> {
    public:
      void do_format(
        const s::const_string_view &,
        const value & v,
        s::styled_sink & out,
        s::style cs
      ) const override {
        switch (v.type) {
        case value_type::none:
          out.write_styled(invalid, "Invalid");
          return;
        case value_type::reg:
          out.write_styled(value_register, v.name);
          return;
        case value_type::address:
          out.set_style(value_address);
          out.write("0x");
          f::integer_formatter_v<u64_t>.do_format("012X", v.as_raw_u64(), out, value_address);
          out.set_style(cs);
          return;
        case value_type::signed_immediate:
          out.set_style(value_immediate);
          f::integer_formatter_v<i64_t>.do_format("", v.as_i64(), out, value_immediate);
          out.set_style(cs);
          return;
        case value_type::unsigned_immediate:
          out.set_style(value_immediate);
          f::integer_formatter_v<u64_t>.do_format("", v.as_u64(), out, value_immediate);
          out.set_style(cs);
          return;
        }
      }

      u32_t do_get_length(const s::const_string_view &, const value & v) const noexcept override {
        switch (v.type) {
        case value_type::none:
          return 7U;
        case value_type::reg:
          return v.name.size();
        case value_type::address:
          return 2U + f::integer_formatter_v<u64_t>.do_get_length("012X", v.as_raw_u64());
        case value_type::signed_immediate:
          return f::integer_formatter_v<i64_t>.do_get_length("", v.as_i64());
        case value_type::unsigned_immediate:
          return f::integer_formatter_v<u64_t>.do_get_length("", v.as_u64());
        }
      }
    };

    J_A(ND, NODESTROY) const value_formatter g_value_formatter;

    class J_TYPE_HIDDEN operand_formatter final : public s::formatter_known_length<operand> {
    public:
      void do_format(const s::const_string_view &,
                     const operand & v,
                     s::styled_sink & out,
                     s::style cs) const override
      {
        if (!v) {
          out.write_styled(invalid, "Invalid");
          return;
        }
        if (v.name) {
          v.name.format(out, cs);
          out.write(" (");
        }
        if (v.type == operand_type::immediate &&
            (v.base.type == value_type::signed_immediate || v.base.type == value_type::unsigned_immediate)) {
          u64_t value = v.base.as_raw_u64();
          out.set_style(value_immediate);
          const char * fmt = "";
          if (v.is_binary()) {
            out.write("0b");
            switch (v.base.width) {
            case value_width::byte:
              fmt = "08b";
              value &= 0xFFU;
              break;
            case value_width::word:
              fmt = "016b";
              value &= 0xFFFFU;
              break;
            case value_width::dword:
              fmt = "032b";
              value &= 0xFFFFFFFFU;
              break;
            default:
              fmt = "064b";
              break;
            }
          } else if (v.is_hex()) {
            out.write("0x");
            switch (v.base.width) {
            case value_width::byte:
              fmt = "02X";
              value &= 0xFFU;
              break;
            case value_width::word:
              fmt = "04X";
              value &= 0xFFFFU;
              break;
            case value_width::dword:
              fmt = "08X";
              value &= 0xFFFFFFFFU;
              break;
            default:
              fmt = "016X";
              break;
            }
          }
          f::integer_formatter_v<u64_t>.do_format(fmt, value, out, value_immediate);
          out.set_style(cs);
          if (v.name) {
            out.write(")");
          }
          return;
        }
        if (v.type == operand_type::memory) {
          out.write_styled(value_brackets, brackets[(u8_t)v.width]);
        }

        g_value_formatter.do_format("", v.base, out, cs);
        if (v.index) {
          out.write(" + ");
          if (v.scale != 1) {
            out.set_style(scale);
            f::integer_formatter_v<i64_t>.do_format("", v.scale, out, scale);
            out.set_style(cs);
            out.write("*");
          }
          g_value_formatter.do_format("", v.index, out, cs);
        }
        auto disp = v.displacement;
        if (disp) {
          out.set_style(displacement);
          if (disp > 0) {
            out.write(" + ");
          } else {
            out.write(" - ");
            disp = -disp;
          }
          f::integer_formatter_v<i64_t>.do_format("", disp, out, displacement);
          out.set_style(cs);
        }
        if (v.type == operand_type::memory) {
          out.write_styled(value_brackets, "]");
        }
        if (v.name) {
          out.write(")");
        }
      }

      u32_t do_get_length(const s::const_string_view &, const operand & v) const noexcept override {
        if (!v) {
          return 7U;
        }
        u32_t result = 0U;
        if (v.name) {
          result += 2U + v.name.get_length();
        }
        if (v.type == operand_type::immediate &&
            (v.base.type == value_type::signed_immediate || v.base.type == value_type::unsigned_immediate)) {
          if (v.is_binary()) {
            switch (v.base.width) {
            case value_width::byte:
              result += 2 + 8;
              break;
            case value_width::word:
              result += 2 + 16;
              break;
            case value_width::dword:
              result += 2 + 32;
              break;
            default:
              result += 2 + 64;
              break;
            }
          } else if (v.is_hex()) {
            switch (v.base.width) {
            case value_width::byte:
              result += 2 + 2;
              break;
            case value_width::word:
              result += 2 + 4;
              break;
            case value_width::dword:
              result += 2 + 8;
              break;
            default:
              result += 2 + 16;
              break;
            }
          } else {
            result += f::integer_formatter_v<u64_t>.do_get_length("", v.base.as_raw_u64());
          }
          if (v.name) {
            result++;
          }
          return result;
        }
        result = g_value_formatter.do_get_length("", v.base);
        if (v.index) {
          result += 3;
          ++result; // " + "
          if (v.scale != 1) {
            result += f::integer_formatter_v<i64_t>.do_get_length("", v.scale);
            ++result; // *
          }
          result += g_value_formatter.do_get_length("", v.index);
        }

        if (v.type == operand_type::memory) {
          result += brackets[(u8_t)v.width].size() + 1U;
        }

        auto disp = v.displacement;
        if (disp) {
          result += 3; // " + " or " - "
          result += f::integer_formatter_v<i64_t>.do_get_length("", disp < 0 ? -disp : disp);
        }
        if (v.name) {
          result++;
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const operand_formatter g_operand_formatter;

    inline constexpr i32_t g_mnemonic_pad = 10U;

    class J_TYPE_HIDDEN instruction_formatter final : public s::formatter_known_length<instruction> {
    public:
      void do_format(const s::const_string_view &,
                     const instruction & v,
                     s::styled_sink & out,
                     s::style cs) const override
      {
        if (v.label) {
          auto style = v.label_offset ? invalid : label;
          out.write_styled(style, v.label);
          if (v.label_offset) {
            f::integer_formatter_v<i64_t>.do_format("", v.label_offset, out, style);
          }
          out.write_styled(style, ":");
          out.write("\n");
        }
        u32_t col = out.get_column();
        if (v.source_location) {
          sources::g_source_location_formatter.do_format("", v.source_location, out, cs);
        }
        u32_t src_pad =  25 - min(24, out.get_column() - col);
        f::g_indent_formatter_u32_t.do_format("", src_pad, out, cs);

        col = out.get_column();
        out.write_styled(mnemonics[(u8_t)v.category], v.mnemonic);
        bool is_first = true;
        for (auto & op : v.get_operands()) {
          if (is_first) {
            if (v.mnemonic.size() >= g_mnemonic_pad) {
              out.write(" ");
            } else {
              out.write("                    ", g_mnemonic_pad - v.mnemonic.size());
            }
            is_first = false;
          } else {
            out.write(", ");
          }
          g_operand_formatter.do_format("", op, out, cs);
        }
        if (v.comment || v.allowed_types) {
          col = out.get_column() - col;
          u32_t comment_pad = col > 36 ? 36 : col;
          out.write_styled(comment,
                           ("                                     ; ") + comment_pad);

          if (v.allowed_types) {
            out.write_styled(comment, "Type check: ");
            g_imm_type_mask_formatter.do_format("", v.allowed_types, out, cs);
          } else {
            out.write_styled(comment, v.comment);
          }
        }
      }

      u32_t do_get_length(const s::const_string_view &, const instruction & v) const noexcept override {
        u32_t result = 0U;
        if (v.label) {
          result += v.label.size() + 1U + 1U;
          if (v.label_offset) {
            result += f::integer_formatter_v<i64_t>.do_get_length("", v.label_offset);
          }
        }
        result += 25U;
        if (v.source_location) {
          result += max(0, sources::g_source_location_formatter.do_get_length("", v.source_location) - 25);
        }

        u32_t subresult = 0U;
        subresult += v.mnemonic.size();
        bool is_first = true;
        for (auto & op : v.get_operands()) {
          if (is_first) {
            if (v.mnemonic.size() >= g_mnemonic_pad) {
              ++subresult;
            } else {
              subresult += g_mnemonic_pad - v.mnemonic.size();
            }
            is_first = false;
          } else {
            subresult += 2U;
          }
          subresult += g_operand_formatter.do_get_length("", op);
        }
        if (v.allowed_types) {
          result += max(36, subresult) + 3 + 12 + g_imm_type_mask_formatter.do_get_length("", v.allowed_types);
        } else if (v.comment) {
          result += max(36, subresult) + 3 + v.comment.size();
        } else {
          result += subresult;
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const instruction_formatter g_instruction_formatter;
  }
}
