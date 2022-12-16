#include "lisp/cir/debug/op_formatter.hpp"

#include "lisp/cir/debug/input_formatter.hpp"
#include "lisp/cir/cir_context.hpp"
#include "lisp/cir/locs/debug_formatters.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/cir/ops/conditions.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "lisp/common/debug_formatters.hpp"
#include "lisp/assembly/target.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "lisp/assembly/register_info.hpp"
#include "lisp/values/type_formatters.hpp"
#include "hzd/type_traits.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/formatters/numbers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "lisp/env/debug_formatters.hpp"

namespace j::lisp::cir::inline debug {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;
    static const f::debug_enum_entry op_type_entries[] = {
      [(u8_t)op_type::none]        = { "none",         s::styles::error},

      [(u8_t)op_type::phi]         = { " φ ",          s::styles::tag4 },
      [(u8_t)op_type::iconst]      = { "%iconst",      s::styles::yellow },
      [(u8_t)op_type::fconst]      = { "%fconst",      s::styles::yellow },
      [(u8_t)op_type::mconst]      = { "%mconst",      s::styles::yellow },

      [(u8_t)op_type::mov]         = { "%mov",         s::styles::bright_cyan },
      [(u8_t)op_type::icopy]       = { "%icopy",       s::styles::light_gray },
      [(u8_t)op_type::fcopy]       = { "%fcopy",       s::styles::light_gray },
      [(u8_t)op_type::push]        = { "%push",        s::styles::bright_yellow },
      [(u8_t)op_type::pop]         = { "%pop",         s::styles::bright_yellow },
      [(u8_t)op_type::swap]        = { "%swap",        s::styles::bright_yellow },

      [(u8_t)op_type::fn_enter]    = { "%fn-enter",    s::styles::gray },
      [(u8_t)op_type::fn_iarg]     = { "%fn-iarg",     s::styles::cyan },
      [(u8_t)op_type::fn_farg]     = { "%fn-farg",     s::styles::bright_cyan },
      [(u8_t)op_type::fn_rest_ptr] = { "%fn-rest-ptr", s::styles::cyan },
      [(u8_t)op_type::fn_rest_len] = { "%fn-rest-len", s::styles::bright_cyan },
      [(u8_t)op_type::fn_sc_ptr]   = { "%fn-sc-ptr",   s::styles::bright_yellow },

      [(u8_t)op_type::alloca]      = { "%alloca",      s::styles::magenta },
      [(u8_t)op_type::dealloca]    = { "%dealloca",    s::styles::magenta },
      [(u8_t)op_type::full_call]   = { "%full-call",   s::styles::tag16 },
      [(u8_t)op_type::abi_call]    = { "%abi-call",    s::styles::tag16 },
      [(u8_t)op_type::call]        = { "%call",        s::styles::tag16 },

      [(u8_t)op_type::ineg]        = { "%ineg",        s::styles::bright_green },
      [(u8_t)op_type::iadd]        = { "%iadd",        s::styles::bright_green },
      [(u8_t)op_type::isub]        = { "%isub",        s::styles::bright_green },
      [(u8_t)op_type::smul]        = { "%smul",        s::styles::bright_green },
      [(u8_t)op_type::sdiv]        = { "%sdiv",        s::styles::bright_green },
      [(u8_t)op_type::umul]        = { "%umul",        s::styles::bright_green },
      [(u8_t)op_type::udiv]        = { "%udiv",        s::styles::bright_green },

      [(u8_t)op_type::fadd]        = { "%fadd",        s::styles::bright_yellow },
      [(u8_t)op_type::fsub]        = { "%fsub",        s::styles::bright_yellow },

      [(u8_t)op_type::shl]         = { "%shl",         s::styles::green },
      [(u8_t)op_type::shr]         = { "%shr",         s::styles::green },
      [(u8_t)op_type::sar]         = { "%sar",         s::styles::green },

      [(u8_t)op_type::band]        = { "%and",         s::styles::bright_green },
      [(u8_t)op_type::bor]         = { "%or",          s::styles::bright_green },
      [(u8_t)op_type::bxor]        = { "%xor",         s::styles::bright_green },

      [(u8_t)op_type::eq]          = { "%eq",          s::styles::bright_green },
      [(u8_t)op_type::neq]         = { "%neq",         s::styles::bright_green },

      [(u8_t)op_type::tag]         = { "%tag",         s::styles::bright_blue },
      [(u8_t)op_type::untag]       = { "%untag",       s::styles::bright_blue },

      [(u8_t)op_type::mem_ird]     = { "%mem-ird",     s::styles::bright_yellow },
      [(u8_t)op_type::mem_iwr]     = { "%mem-iwr",     s::styles::bright_red },
      [(u8_t)op_type::mem_frd]     = { "%mem-frd",     s::styles::bright_yellow },
      [(u8_t)op_type::mem_fwr]     = { "%mem-fwr",     s::styles::bright_red },

      [(u8_t)op_type::mem_copy64] = { "%mem-copy64", s::styles::bright_red },

      [(u8_t)op_type::lea]     = { "%lea",     s::styles::bright_green },

      [(u8_t)op_type::debug_trap]  = { "%%debug-trap",    s::styles::yellow_on_red },

      [(u8_t)op_type::iret]        = { "%iret",        s::styles::light_gray },
      [(u8_t)op_type::fret]        = { "%fret",        s::styles::bright_yellow },
      [(u8_t)op_type::type_error]  = { "%%type-error", s::styles::error },

      [(u8_t)op_type::jmp]         = { "%jmp",         s::styles::black_on_yellow },
      [(u8_t)op_type::jmpt]        = { "%jmpt",        s::styles::black_on_yellow },
      [(u8_t)op_type::jmpc]        = { "%jmpc",        s::styles::black_on_yellow },
      [(u8_t)op_type::jmptype]     = { "%jmptype",     s::styles::black_on_yellow },
    };

    J_A(ND, NODESTROY) const f::debug_enum_formatter<op_type> op_type_formatter(op_type_entries);

    static const f::debug_enum_entry cond_entries[] = {
      [(u8_t)condition::eq]      = { "eq" },
      [(u8_t)condition::neq]     = { "neq" },

      [(u8_t)condition::ult]     = { "ult",     s::styles::bright_green },
      [(u8_t)condition::ugt]     = { "ugt",     s::styles::bright_green },
      [(u8_t)condition::ulte]    = { "ulte",    s::styles::bright_green },
      [(u8_t)condition::ugte]    = { "ugte",    s::styles::bright_green },

      [(u8_t)condition::slt]     = { "slt",     s::styles::bright_cyan },
      [(u8_t)condition::sgt]     = { "sgt",     s::styles::bright_cyan },
      [(u8_t)condition::slte]    = { "slte",    s::styles::bright_cyan },
      [(u8_t)condition::sgte]    = { "sgte",    s::styles::bright_cyan },

      [(u8_t)condition::feq]     = { "feq",     s::styles::bright_yellow },
      [(u8_t)condition::fneq]    = { "fneq",    s::styles::bright_yellow },
      [(u8_t)condition::flt]     = { "flt",     s::styles::bright_yellow },
      [(u8_t)condition::fgt]     = { "fgt",     s::styles::bright_yellow },
      [(u8_t)condition::flte]    = { "flte",    s::styles::bright_yellow },
      [(u8_t)condition::fgte]    = { "fgte",    s::styles::bright_yellow },

      [(u8_t)condition::band]    = { "band",    s::styles::bright_green },
      [(u8_t)condition::bnand]   = { "bnand",   s::styles::bright_green },
      [(u8_t)condition::isset]   = { "isset",   s::styles::bright_green },
      [(u8_t)condition::isclear] = { "isclear", s::styles::bright_green },
    };

    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<condition> condition_formatter(cond_entries);

    class J_HIDDEN cir_op_index_formatter final : public s::formatter_known_length<op_index> {
    public:
      void do_format(
        const s::const_string_view & opts,
        const op_index & o,
        s::styled_sink & to,
        s::style cs
      ) const override {
        if (!o) {
          to.write("?");
          return;
        }

        s::formatters::integer_formatter_v<i16_t>.do_format(opts, o.instruction_index(), to, cs);
        to.write(o.is_pre() ? "B" : "A");
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & opts,
        const op_index & o
      ) const noexcept override {
        if (!o) {
          return 1;
        }
        return s::formatters::integer_formatter_v<i16_t>.do_get_length(opts, o.instruction_index()) + 1;
      }
    };

    J_A(ND, NODESTROY) const static cir_op_index_formatter cir_op_index_formatter_v;

    [[nodiscard]] static u32_t get_op_result_length(const op * def, loc loc) noexcept{
      if (J_UNLIKELY(!def)) {
        return 8U;
      }
      u32_t result = 1U + cir_op_index_formatter_v.do_get_length("", def->index);
      if (loc) {
        result += 1U + g_loc_formatter.do_get_length("", loc);
      }
      return result;
    }

    [[nodiscard]] u32_t get_operand_length(loc value) noexcept {
      return 2U + g_loc_formatter.do_get_length("", value);
    }

    [[nodiscard]] u32_t get_operand_length(const op_result_input & d) noexcept {
      return get_operand_length(d.use.loc_in);
    }

    [[nodiscard]] u32_t get_operand_length(const bb * J_NOT_NULL exit) noexcept {
      return 2U + exit->name.size();
    }

    [[nodiscard]] u32_t get_operand_length(mem_width w) noexcept {
      return g_mem_width_formatter.do_get_length("", w);
    }

    [[nodiscard]] u32_t get_operand_length(const j::mem::const_memory_region & r) noexcept {
      return 2U + s::formatters::integer_formatter_v<u64_t>.do_get_length("", r.size());
    }

    [[nodiscard]] u32_t get_operand_length(const imm_type & value) noexcept {
      return 2U + g_imm_type_formatter.do_get_length("", value);
    }

    [[nodiscard]] u32_t get_operand_length(const imm_type_mask & value) noexcept {
      return 2U + g_imm_type_mask_formatter.do_get_length("", value);
    }

    [[nodiscard]] u32_t get_operand_length(condition value) noexcept {
      return 2U + condition_formatter.do_get_length("", value);
    }

    [[nodiscard]] u32_t get_operand_length(const input & value) noexcept {
      return 2U + g_input_formatter.do_get_length("", value);
    }

    [[nodiscard]] u32_t get_operand_length(bool value) noexcept {
      return value ? 6U : 7U;
    }

    template<Arithmetic T>
    [[nodiscard]] u32_t get_operand_length(T value) noexcept {
      return 2U + s::formatters::number_formatter_v<T>.do_get_length("", value);
    }

    [[nodiscard]] u32_t get_operand_length(const span<const input> & value) noexcept {
      u32_t result = 0U;
      for (auto & input : value) {
        result += get_operand_length(input);
      }
      return result;
    }

    struct J_TYPE_HIDDEN op_formatter_visitor final {
      s::styled_sink & to;
      s::style cs;
      bool compact = false;
      bool is_first = true;

      void write_separator() {
        to.write(is_first ? "  " : ", ");
        is_first = false;
      }

      void do_format_operand(mem_width value) {
        g_mem_width_formatter.do_format("", value, to, cs);
      }

      void do_format_operand(const j::mem::const_memory_region & r) {
        s::formatters::integer_formatter_v<u64_t>.do_format("", r.size(), to, cs);
      }

      void do_format_operand(imm_type value) {
        g_imm_type_formatter.do_format("", value, to, cs);
      }

      void do_format_operand(const imm_type_mask & value) {
        g_imm_type_mask_formatter.do_format("", value, to, cs);
      }

      void do_format_operand(condition value) {
        condition_formatter.do_format("", value, to, cs);
      }

      void do_format_operand(loc value) {
        g_loc_formatter.do_format("", value, to, cs);
      }

      void do_format_operand(const input & value) {
        g_input_formatter.do_format("", value, to, cs);
      }

      void do_format_operand(const bb * J_NOT_NULL exit) {
        to.write(exit->name);
      }

      void do_format_operand(bool val) {
        to.write(val ? "true" : "false");
      }

      template<Arithmetic T>
      void do_format_operand(T value) {
        s::formatters::number_formatter_v<T>.do_format("", value, to, cs);
      }
      template<typename T>
      void format_operand(const T & value) {
        write_separator();
        do_format_operand(value);
      }

      void format_operand(const span<const input> & value) {
        for (auto & input : value) {
          write_separator();
          g_input_formatter.do_format("", input, to, cs);
        }
      }

      void format_op_type(const op * o) {
        auto col = to.get_column();
        op_type_formatter.do_format("", o->type, to, cs);
        if (!compact) {
          to.write(("            ") + (to.get_column() - col));
        }
      }

      void op_header(const op * o) {
        if (!compact) {
          to.write("            ");
        }
        format_op_type(o);
      }

      template<typename Type, typename... Opers>
      void operator()(Type, const op * o, Opers && ... operands) {
        op_header(o);
        (format_operand(static_cast<Opers &&>(operands)), ...);
      }

      void op_header(const op * o, loc result) {
        if (J_UNLIKELY(!o)) {
          to.write_styled(s::styles::error, "detached");
          return;
        }
        to.write("$");
        cir_op_index_formatter_v.do_format("", o->index, to, cs);

        if (result) {
          to.write_styled(s::styles::bright_cyan, "@");
          g_loc_formatter.do_format("", result, to, cs);
        }

        u32_t len = get_op_result_length(o, result);
        to.write(compact ? " = " : (("          = ") + min(len, 10U)));
        format_op_type(o);
      }

      template<typename Type, typename... Opers>
      void operator()(Type, const op * o, loc result, Opers && ... operands) {
        op_header(o, result);
        (format_operand(static_cast<Opers &&>(operands)), ...);
      }
    };

    struct J_TYPE_HIDDEN op_size_visitor final {
      bool compact = false;

      template<typename Type, typename... Opers>
      u32_t operator()(Type, const op * o, Opers && ... operands) const noexcept {
        return (get_operand_length(static_cast<Opers &&>(operands)) + ... + (compact ? op_type_formatter.do_get_length("", o->type) : 24U));
      }

      template<typename Type, typename... Opers>
      u32_t operator()(Type, const op * J_NOT_NULL o, loc result, Opers && ... operands) const noexcept {
        u32_t len = (get_operand_length(static_cast<Opers &&>(operands)) + ... + 0U);
        return len
          + min(get_op_result_length(o, result) + 2U, compact ? 0U : 12U)
          + (compact ? op_type_formatter.do_get_length("", o->type) : 12U);
      }
    };

    class J_TYPE_HIDDEN cir_op_formatter final : public s::formatter_known_length<op> {
    public:

      void do_format(
        const s::const_string_view & opts,
        const op & o,
        s::styled_sink & to,
        s::style cs
      ) const override {
        op_formatter_visitor v{to, cs, opts == "compact"};
        visit_cir_op(&o, v);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & opts,
        const op & o
      ) const noexcept override {
        op_size_visitor v{.compact = (opts == "compact")};
        return visit_cir_op(&o, v);
      }
    };

    J_A(ND, NODESTROY) const static cir_op_formatter cir_op_formatter_v;
  }

  const strings::formatter_known_length<op_type> & g_op_type_formatter = op_type_formatter;
  const strings::formatter_known_length<op_index> & g_op_index_formatter = cir_op_index_formatter_v;
}
