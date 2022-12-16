#include "lisp/air/debug/debug_formatters.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"
#include "lisp/air/values/val_spec.hpp"
#include "lisp/values/type_formatters.hpp"
#include "strings/styling/default_styles.hpp"
#include "lisp/air/values/rep.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::air {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    J_A(ND, NODESTROY) const f::debug_enum_entry reps[] = {
      [rep_none]        = { "N", s::styles::error },
      [rep_tagged]      = { "T", s::styles::green },
      [rep_untagged]    = { "U", s::styles::yellow },
      [rep_range]       = { "R", s::styles::bright_magenta },
    };

    J_A(ND, NODESTROY) const f::debug_enum_entry reps_long[] = {
      [rep_none]        = { "None", s::styles::error },
      [rep_tagged]      = { "Tagged", s::styles::green },
      [rep_untagged]    = { "Untagged", s::styles::yellow },
      [rep_range]       = { "Range", s::styles::bright_magenta },
    };

    J_A(ND, NODESTROY) const f::debug_enum_formatter<rep> rep_formatter(reps);
    J_A(ND, NODESTROY) const f::debug_enum_formatter<rep> rep_formatter_long(reps_long, "long");


    class J_TYPE_HIDDEN rep_spec_formatter final : public s::formatter_known_length<rep_spec> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const rep_spec & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        if (v.has_rep()) {
          to.write("!");
          bool is_long = fo == "long";
          (is_long ? rep_formatter_long : rep_formatter).do_format("", v.rep(), to, cs);
          return;
        }
        if (!v.mask()) {
          to.write_styled(s::styles::error, "None");
          return;
        }
        u8_t i = 0;
        for (auto & r : reps) {
          if (v.has((rep)i)) {
            to.write_styled(r.style, r.name);
          }
          ++i;
        }
        if (v.hint() && v.mask() != v.hint()) {
          to.write_styled(s::styles::bright_green, "+");
          i = 0U;
          for (auto & r : reps) {
            if (v.has_hint((rep)i)) {
              to.write_styled(r.style, r.name);
            }
            ++i;
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & fo,
        const rep_spec & v
      ) const noexcept override {
        if (v.has_rep()) {
          bool is_long = fo == "long";
          return 1U + (is_long ? rep_formatter_long : rep_formatter).do_get_length("", v.rep());
        }
        if (!v.mask()) {
          return 4U;
        }
        u32_t result = bits::popcount(v.mask());
        if (v.hint() && v.hint() != v.mask()) {
          result += bits::popcount(v.hint()) + 1U;
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const rep_spec_formatter rep_spec_fmt_v;



    struct J_TYPE_HIDDEN val_special_case final {
      enum rep rep;
      val_spec spec;
      s::const_string_view name;
      s::style style;
    };

    inline const val_special_case val_spec_specials[]{
      {rep_none,     val_none,                                               "None",    s::styles::error},
      {rep_none,     val_any,                                                "Any",     s::styles::bright_yellow},
      {rep_none,     val_imm,                                                "Imm",     s::styles::bright_green},
      {rep_untagged, {imm_act_rec, rep_spec::untagged},                      "ActRec",  s::styles::bright_red},
      {rep_none,     {{imm_act_rec, imm_nil}, rep_spec::tagged_or_untagged}, "ActRec?", s::styles::bright_red},
      {rep_tagged,   {imm_bool,   rep_spec::tagged},                         "Bool",    s::styles::bright_blue},
      {rep_tagged,   {imm_sym_id, rep_spec::tagged},                         "Sym",     s::styles::bright_cyan},
      {rep_tagged,   {imm_nil,    rep_spec::tagged},                         "Nil",     s::styles::red},
      {rep_range,    {imm_range,  rep_spec::range},                          "Range",   s::styles::bright_magenta},
      {rep_range,    {{imm_vec_ref, imm_range}, rep_spec::range},            "VecRng",  s::styles::magenta},
      {rep_none,     {{imm_vec_ref, imm_range}, rep_spec::any},              "AnySeq",  s::styles::bright_magenta},
    };

    inline const val_special_case * find_special_case(val_spec v) noexcept {
      if (!v.reps.has_rep() && v.reps.hint() == v.reps.mask()) {
        v.reps.set_hint(0U);
      }
      for (auto & s : val_spec_specials) {
        if (v.reps.has_rep() ? (v.rep() == s.rep && v.types == s.spec.types) : (v == s.spec)) {
          return &s;
        }
      }
      return nullptr;
    }

    class J_TYPE_HIDDEN val_spec_formatter final : public s::formatter_known_length<val_spec> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const val_spec & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        if (auto s = find_special_case(v)) {
          to.write_styled(s->style, s->name);
          return;
        }

        rep_spec_fmt_v.do_format(fo, v.reps, to, cs);
        to.write(" ");
        g_imm_type_mask_formatter.do_format("", v.types, to, cs);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & fo,
        const val_spec & v
      ) const noexcept override {
        if (auto s = find_special_case(v)) {
          return s->name.size();
        }
        return rep_spec_fmt_v.do_get_length(fo, v.reps)
          + 1U + g_imm_type_mask_formatter.do_get_length("", v.types);
      }
    };

    J_A(ND, NODESTROY) const val_spec_formatter val_spec_fmt_v;
  }
  const strings::formatter_known_length<val_spec> & g_val_spec_formatter = val_spec_fmt_v;
}
