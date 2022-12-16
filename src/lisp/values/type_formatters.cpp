#include "lisp/values/type_formatters.hpp"

#include "strings/styling/default_styles.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::inline values {
  namespace s = strings;
  namespace f = s::formatters;

  namespace {
    const f::debug_enum_entry imm_tag_entries[] = {
      [(u8_t)tag_i64]      = { "i64", s::styles::white },
      [(u8_t)tag_f32]      = { "f32", s::styles::bright_green },
      [(u8_t)tag_bool]  = { "bool", s::styles::bright_cyan },
      [(u8_t)tag_nil]      = { "nil", s::styles::bright_cyan },
      [(u8_t)tag_fn_ref] = { "fn", s::styles::bright_yellow },
      [(u8_t)tag_closure_ref] = { "bfn", s::styles::bright_red },
      [(u8_t)tag_str_ref]  = { "str", s::styles::bright_blue },
      [(u8_t)tag_sym_id]   = { "sym", s::styles::bright_cyan },
      [(u8_t)tag_vec_ref]  = { "vec", s::styles::bright_magenta },
      [(u8_t)tag_undefined]  = { "UNDEF", s::styles::error },
    };

    const f::debug_enum_entry imm_type_entries[] = {
      [(u8_t)imm_i64]         = { "I64", s::styles::white },
      [(u8_t)imm_f32]         = { "F32", s::styles::bright_green },
      [(u8_t)imm_bool]        = { "Bool", s::styles::bright_cyan },
      [(u8_t)imm_nil]         = { "Nil", s::styles::bright_cyan },
      [(u8_t)imm_fn_ref]      = { "Fn", s::styles::bright_yellow },
      [(u8_t)imm_closure_ref] = { "BFn", s::styles::bright_red },
      [(u8_t)imm_str_ref]     = { "Str", s::styles::bright_blue },
      [(u8_t)imm_sym_id]      = { "Sym", s::styles::bright_cyan },
      [(u8_t)imm_vec_ref]     = { "Vec", s::styles::bright_magenta },
      [(u8_t)imm_rec_ref]     = { "Rec", s::styles::bright_red },
      [(u8_t)imm_act_rec]     = { "Act", s::styles::bright_red },
      [(u8_t)imm_range]       = { "Range", s::styles::bright_magenta },
      [(u8_t)imm_undefined]       = { "UNDEF", s::styles::error },
    };
    J_A(ND, NODESTROY) const f::debug_enum_formatter<imm_tag> imm_tag_formatter(imm_tag_entries);
    J_A(ND, NODESTROY) const f::debug_enum_formatter<imm_type> imm_type_formatter(imm_type_entries);

    struct J_TYPE_HIDDEN imm_mask_special_case final {
      imm_type_mask mask;
      s::const_string_view name;
      s::style style;
    };

    const imm_mask_special_case imm_mask_specials[]{
      { {}, "None", s::styles::error },
      { numeric_imm_type, "Num", s::styles::bold },
      { taggable_imm_type, "Imm", s::styles::bright_green },
      { any_imm_type, "Any", s::styles::bright_yellow },
    };

    struct J_TYPE_HIDDEN imm_mask_formatter final : public s::formatter_known_length<imm_type_mask> {
      void do_format(
        const s::const_string_view &,
        const imm_type_mask & v,
        s::styled_sink & to,
        s::style cs
        ) const override
      {
        for (auto & c : imm_mask_specials) {
          if (c.mask == v) {
            to.write_styled(c.style, c.name);
            return;
          }
        }
        bool is_first = true;
        for (imm_type idx : v) {
          if (!is_first) {
            to.write("|");
          }
          is_first = false;
          imm_type_formatter.do_format("", idx, to, cs);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const imm_type_mask & v
        ) const noexcept override
      {
        for (auto & c : imm_mask_specials) {
          if (c.mask == v) {
            return c.name.size();
          }
        }

        u32_t result = 0U;
        for (imm_type idx : v) {
          ++result;
          result += imm_type_formatter.do_get_length("", idx);
        }
        return result - 1U;
      }
    };

    J_A(ND, NODESTROY) const imm_mask_formatter imm_mask_formatter_v;
  }

  const strings::formatter_known_length<imm_type> & g_imm_type_formatter = imm_type_formatter;
  const strings::formatter_known_length<imm_type_mask> & g_imm_type_mask_formatter = imm_mask_formatter_v;
}
