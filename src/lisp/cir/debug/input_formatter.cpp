#include "input_formatter.hpp"

#include "lisp/cir/debug/op_formatter.hpp"
#include "lisp/cir/ops/op.hpp"
#include "lisp/cir/ops/atomic_inputs.hpp"
#include "lisp/cir/ops/mem_input.hpp"
#include "lisp/cir/locs/debug_formatters.hpp"
#include "lisp/env/debug_formatters.hpp"
#include "lisp/common/debug_formatters.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/formatters/numbers.hpp"

namespace j::lisp::cir::inline debug {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    static const f::debug_enum_entry reloc_type_entries[] = {
      [(u8_t)reloc_type::none]               = { "None", s::styles::bright_red },
      [(u8_t)reloc_type::constant_addr]      = { "ConstPtr", s::styles::light_gray.with_bold() },
      [(u8_t)reloc_type::fn_addr]            = { "FnPtr", s::styles::bright_magenta },
      [(u8_t)reloc_type::var_addr]           = { "VarPtr", s::styles::bright_cyan },
      [(u8_t)reloc_type::var_value_untagged] = { "Untagged", s::styles::bright_red },
      [(u8_t)reloc_type::var_value_tagged]   = { "Tagged", s::styles::bright_red },
    };

    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<reloc_type> reloc_type_formatter(reloc_type_entries);

    class op_result_input_formatter final : public s::formatter_known_length<op_result_input> {
    public:
      void do_format(const s::const_string_view &, const op_result_input & v, s::styled_sink & to, s::style cs) const override {
        if (!v) {
          to.write_styled(s::styles::bright_red, "None");
          return;
        } else if (!v.use.def->index) {
          to.write_styled(s::styles::bright_yellow, "$?");
          return;
        }
        to.write_styled(s::styles::bright_cyan, "$");
        g_op_index_formatter.do_format("", v.use.def->index, to, cs);
        if (v.use.loc_in || v.use.loc_out) {
          to.write_styled(s::styles::bright_yellow, "[");
          g_loc_formatter.do_format("", v.use.loc_in, to, cs);
          if (v.use.loc_out != v.use.loc_in) {
            to.write_styled(s::styles::bold, "->");
            g_loc_formatter.do_format("", v.use.loc_out, to, cs);
          }
          to.write_styled(s::styles::bright_yellow, "]");
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const op_result_input & v) const noexcept override {
        if (!v) {
          return 4U;
        }
        if (!v.use.def->index) {
          return 2U;
        }
        u32_t result = 1U + g_op_index_formatter.do_get_length("", v.use.def->index);

        if (v.use.loc_in || v.use.loc_out) {
          result += 1U + g_loc_formatter.do_get_length("", v.use.loc_in);
          if (v.use.loc_out != v.use.loc_in) {
            result += 2U + g_loc_formatter.do_get_length("", v.use.loc_out);
          }
          result += 1U;
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const op_result_input_formatter op_result_fmt;

    class reloc_input_formatter final : public s::formatter_known_length<reloc_input> {
    public:
      void do_format(const s::const_string_view &, const reloc_input & v, s::styled_sink & to, s::style cs) const override {
        if (v.reloc_type == reloc_type::fn_addr) {
          to.write_styled(s::styles::bright_magenta, functions::cc_titles[(u8_t)v.calling_convention]);
          to.write_styled(s::styles::bright_magenta, "Ptr");
        } else {
          reloc_type_formatter.do_format("", v.reloc_type, to, cs);
        }
        if (v.reloc_type != reloc_type::none) {
          to.write(" ");
          if (v.reloc_type == reloc_type::constant_addr) {
            f::integer_formatter_v<u32_t>.do_format("04X", v.constant_offset, to, cs);
          }  else {
            env::g_id_formatter.do_format("", v.id, to, cs);
          }
          if (v.addend) {
            to.write("+");
            f::integer_formatter_v<i32_t>.do_format("", v.addend, to, cs);
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const reloc_input & v) const noexcept override {
        u32_t len = 0U;
        if (v.reloc_type == reloc_type::fn_addr) {
          len = functions::cc_titles[(u8_t)v.calling_convention].size() + 3U;
        } else {
          len = reloc_type_formatter.do_get_length("", v.reloc_type);
        }
        return len
          + (v.reloc_type != reloc_type::none
             ? 1U + (v.reloc_type == reloc_type::constant_addr
                     ? f::integer_formatter_v<u32_t>.do_get_length("04X", v.constant_offset)
                     : env::g_id_formatter.do_get_length("", v.id))
             : 0U)
          + (v.addend ? 1U + f::integer_formatter_v<i32_t>.do_get_length("", v.addend) : 0U);
      }
    };

    J_A(ND, NODESTROY) const reloc_input_formatter reloc_fmt;

    class atomic_input_formatter final : public s::formatter_known_length<atomic_input> {
    public:
      void do_format(const s::const_string_view &, const atomic_input & v, s::styled_sink & to, s::style cs) const override {
        switch (v.type) {
        case atomic_input_type::none:
          to.write_styled(s::styles::bright_red, "None");
          return;
        case atomic_input_type::op_result:
          op_result_fmt.do_format("", v.op_result_data, to, cs);
          return;
        case atomic_input_type::reloc:
          reloc_fmt.do_format("", v.reloc_data, to, cs);
          return;
        case atomic_input_type::constant:
          f::integer_formatter_v<u64_t>.do_format("04X", v.const_data, to, cs);
          return;
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const atomic_input & v) const noexcept override {
        switch (v.type) {
        case atomic_input_type::none:
          return 4U;
        case atomic_input_type::op_result:
          return op_result_fmt.do_get_length("", v.op_result_data);
        case atomic_input_type::reloc:
          return reloc_fmt.do_get_length("", v.reloc_data);
        case atomic_input_type::constant:
          return f::integer_formatter_v<u64_t>.do_get_length("04X", v.const_data);
        }
      }
    };

    J_A(ND, NODESTROY) const atomic_input_formatter atomic_fmt;

    class mem_input_formatter final : public s::formatter_known_length<mem_input> {
    public:
      void do_format(const s::const_string_view &, const mem_input & v, s::styled_sink & to, s::style cs) const override {
        if (!v) {
          to.write_styled(s::styles::bright_red, "Empty");
          return;
        }
        g_mem_width_formatter.do_format("", v.width, to, cs);
        to.write("@[");
        atomic_fmt.do_format("", v.base, to, cs);
        if (v.index) {
          to.write(" + ");
          atomic_fmt.do_format("", v.index, to, cs);
          to.write(" ");
          g_mem_scale_formatter.do_format("", normalize_scale(v.scale, v.width), to, cs);
        }
        if (v.displacement) {
          to.write(" + ");
          f::integer_formatter_v<i32_t>.do_format("X", v.displacement, to, cs);
        }
        to.write("]");
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const mem_input & v) const noexcept override {
        if (!v) {
          return 5U;
        }
        u32_t result = g_mem_width_formatter.do_get_length("", v.width)
          + 2U + atomic_fmt.do_get_length("", v.base);
        if (v.index) {
          result += 3U + atomic_fmt.do_get_length("", v.index) + 1U
            + g_mem_scale_formatter.do_get_length("", normalize_scale(v.scale, v.width));
        }
        if (v.displacement) {
          result += 3U + f::integer_formatter_v<i32_t>.do_get_length("X", v.displacement);
        }
        return result + 1U;
      }
    };

    J_A(ND, NODESTROY) const mem_input_formatter mem_fmt;

    class input_formatter final : public s::formatter_known_length<input> {
    public:
      void do_format(const s::const_string_view &, const input & v, s::styled_sink & to, s::style cs) const override {
        switch (v.type) {
        case input_type::none:
          to.write_styled(s::styles::bright_red, "None");
          return;
        case input_type::op_result:
          op_result_fmt.do_format("", v.op_result_data, to, cs);
          return;
        case input_type::reloc:
          reloc_fmt.do_format("", v.reloc_data, to, cs);
          return;
        case input_type::constant:
          f::integer_formatter_v<u64_t>.do_format("04X", v.const_data, to, cs);
          return;
        case input_type::mem:
          mem_fmt.do_format("", v.mem_data, to, cs);
          return;
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const input & v) const noexcept override {
        switch (v.type) {
        case input_type::none:
          return 4U;
        case input_type::op_result:
          return op_result_fmt.do_get_length("", v.op_result_data);
        case input_type::reloc:
          return reloc_fmt.do_get_length("", v.reloc_data);
        case input_type::constant:
          return f::integer_formatter_v<u64_t>.do_get_length("04X", v.const_data);
        case input_type::mem:
          return mem_fmt.do_get_length("", v.mem_data);
        }
      }
    };

    J_A(ND, NODESTROY) const input_formatter input_fmt;
  }

  const strings::formatter_known_length<op_result_input> & g_op_result_type_formatter = op_result_fmt;
  const strings::formatter_known_length<reloc_input> & g_op_reloc_input_formatter = reloc_fmt;
  const strings::formatter_known_length<atomic_input> & g_atomic_input_formatter = atomic_fmt;
  const strings::formatter_known_length<mem_input> & g_mem_input_formatter = mem_fmt;
  const strings::formatter_known_length<input> & g_input_formatter = input_fmt;
}
