#include "lisp/cir/locs/debug_formatters.hpp"
#include "lisp/cir/locs/loc.hpp"
#include "lisp/cir/locs/loc_mask.hpp"
#include "lisp/cir/cir_context.hpp"
#include "lisp/assembly/target.hpp"
#include "lisp/assembly/asm_context.hpp"
#include "lisp/assembly/register_info.hpp"
#include "colors/default_colors.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::cir::inline locs {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;
    namespace c = colors::colors;

    class loc_formatter final : public s::formatter_known_length<loc> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const loc & value,
        s::styled_sink & target,
        s::style current_style
        ) const override {
        switch (value.type()) {
        case loc_type::none:
          target.write("none");
          break;
        case loc_type::spill_slot:
          target.write("S[");
          f::integer_formatter_v<u32_t>.do_format(fo, (u32_t)value.spill_slot_index(), target, current_style);
          target.write("]");
          break;
        case loc_type::gpr:
        case loc_type::fp_reg:
          if (assembly::asm_context->target) {
            target.write(assembly::asm_context->target->get_reg_info()->name_of(value.as_phys_reg()));
            return;
          } else {
            target.write(value.is_fp() ? "$F" : "$I");
            f::integer_formatter_v<u32_t>.do_format(fo, (u32_t)value.index, target, current_style);
          }
          break;
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & fo,
        const loc & value
        ) const noexcept override {
        switch (value.type()) {
        case loc_type::none:
          return 4U;
        case loc_type::spill_slot:
          return 3U + f::integer_formatter_v<u32_t>.do_get_length(fo, (u32_t)value.spill_slot_index());
        case loc_type::fp_reg:
        case loc_type::gpr:
          if (assembly::asm_context->target) {
            return ::j::strlen(assembly::asm_context->target->get_reg_info()->name_of(value.as_phys_reg()));
          }
          return 2U + f::integer_formatter_v<u32_t>.do_get_length(fo, (u32_t)value.index);
        }
      }
    };

    J_A(ND, NODESTROY) const loc_formatter loc_fmt;

    struct J_TYPE_HIDDEN loc_mask_formatter final : public s::formatter_known_length<loc_mask> {
      void do_format(
        const s::const_string_view &,
        const loc_mask & v,
        s::styled_sink & to,
        s::style cs
        ) const override
      {
        u8_t sz = v.size();
        if (sz == 0) {
          to.write("None");
        } else if (sz == 32) {
          to.write("Any");
        } else if (v == loc_mask::any_gpr) {
          to.write("Any GPR");
        } else if (v == loc_mask::any_fp) {
          to.write("Any FP");
        } else {
          loc_mask cur = v;
          if (sz >= 8) {
            bool excludes_fp = cur.covers(loc_mask::any_fp);
            bool excludes_gpr = cur.covers(loc_mask::any_gpr);
            if (excludes_fp) {
              to.write("Any GPR but ");
              cur = ~cur - loc_mask::any_fp;
            } else if (excludes_gpr) {
              to.write("Any FP but ");
              cur = ~cur - loc_mask::any_gpr;
            }
          }

          bool is_first = true;
          for (u16_t i = 0; i < 32; ++i) {
            if (!(cur.mask & (1U << i))) {
              continue;
            }
            if (!is_first) {
              to.write(", ");
            }
            loc_fmt.do_format("", loc::from_raw_u16(i), to, cs);
            is_first = false;
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const loc_mask & v
        ) const noexcept override
      {
        const u8_t sz = v.size();
        if (sz == 0) {
          return 4;
        } else if (sz == 32) {
          return 3;
        } else if (v == loc_mask::any_gpr) {
          return 7;
        } else if (v == loc_mask::any_fp) {
          return 6;
        } else {
          u32_t result = 0;
          loc_mask cur = v;
          bool is_first = true;
          if (sz >= 8) {
            bool excludes_fp = cur.covers(loc_mask::any_fp);
            bool excludes_gpr = cur.covers(loc_mask::any_gpr);
            if (excludes_fp) {
              result += 12;
              cur = ~cur - loc_mask::any_fp;
            } else if (excludes_gpr) {
              result += 11;
              cur = ~cur - loc_mask::any_gpr;
            }
          }
          for (u16_t i = 0; i < 32; ++i) {
            if (!(cur.mask & (1U << i))) {
              continue;
            }
            if (!is_first) {
              result += 2U;
            }
            result += loc_fmt.do_get_length("", loc::from_raw_u16(i));
            is_first = false;
          }
          return result;
        }
      }
    };
    J_A(ND, NODESTROY) const loc_mask_formatter loc_mask_fmt;

    struct J_TYPE_HIDDEN loc_specifier_formatter final : public s::formatter_known_length<loc_specifier> {
      void do_format(
        const s::const_string_view &,
        const loc_specifier & v,
        s::styled_sink & to,
        s::style cs
        ) const override
      {
        to.write("Req: ");
        loc_mask_fmt.do_format("", v.get_mask(false), to, cs);

        if (v.mask != v.hint) {
          to.write("Hint: ");
          loc_mask_fmt.do_format("", v.get_hint(false), to, cs);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const loc_specifier & v
        ) const noexcept override
      {
        u32_t result = 5U + loc_mask_fmt.do_get_length("", v.get_mask(false));
        if (v.mask != v.hint) {
          result += 6U + loc_mask_fmt.do_get_length("", v.get_hint(false));
        }
        return result;
      }
    };
    J_A(ND, NODESTROY) const loc_specifier_formatter loc_specifier_fmt;
  }

  const strings::formatter_known_length<loc> & g_loc_formatter = loc_fmt;
  const strings::formatter_known_length<loc_mask> & g_loc_mask_formatter = loc_mask_fmt;
  const strings::formatter_known_length<loc_specifier> & g_loc_specifier_formatter = loc_specifier_fmt;
}
