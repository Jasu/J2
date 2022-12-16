#include "lisp/values/debug_formatters.hpp"

#include "lisp/values/lisp_object.hpp"
#include "lisp/values/lisp_vec.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "lisp/values/lisp_str.hpp"
#include "lisp/values/lisp_imms.hpp"

#include "lisp/env/debug_formatters.hpp"

#include "strings/formatting/formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/formatters/strings.hpp"
#include "strings/formatters/floats.hpp"
#include "strings/formatters/pointers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::inline values {
  namespace s = strings;
  namespace st = s::styling::styles;
  namespace f = s::formatters;
  namespace e = j::lisp::env;

  class lisp_imm_formatter final : public s::formatter_known_length<
    lisp_imm,
    lisp_f32, lisp_i64,
    lisp_nil, lisp_bool,
    lisp_fn_ref,
    lisp_sym_id,
    lisp_str_ref,
    lisp_vec_ref
  > {
  public:
    void do_format(
      const s::const_string_view & fo,
      const lisp_imm & v,
      s::styled_sink & to,
      s::style cs
    ) const override {
      switch (v.tag()) {
      case tag_i64:
        f::integer_formatter_v<i64_t>.do_format("", v.as_i64().value(), to, cs);
        return;
      case tag_f32:
        f::float_formatter_v<float>.do_format("", v.as_f32().value(), to, cs);
        return;

      case tag_nil:
        to.write_styled(s::styles::light_gray, "nil");
        return;

      case tag_bool:
        to.write_styled(v.as_bool().value() ? s::styles::bright_green : s::styles::bright_red,
                        v.as_bool().value() ? "true" : "false");
        return;

      case tag_str_ref:
        to.write("\"");
        to.write(v.as_str_ref()->value());
        to.write("\"");
        return;
      case tag_vec_ref: {
        bool is_first = true;
        if (fo != "omit-outer-parens") {
          to.write("(");
        }
        for (lisp_imm imm : v.as_vec_ref()->value()) {
          if (!is_first) {
            to.write(" ");
          } else {
            is_first = false;
          }
          do_format("", imm, to, cs);
        }
        if (fo != "omit-outer-parens") {
          to.write(")");
        }
        return;
      }
      case tag_sym_id:
        e::g_id_formatter.do_format("", v.as_sym_id().value(), to, cs);
        return;
      case tag_fn_ref:
        to.write("Fn@");
        f::integer_formatter_v<u64_t>.do_format("012X",
                                       (uptr_t)v.as_fn_ref().value(),
                                       to, cs);
        return;
      case tag_closure_ref:
        to.write("Closure@");
        f::integer_formatter_v<u64_t>.do_format("012X",
                                       (uptr_t)v.as_closure_ref().ptr(),
                                       to, cs);
        to.write("Fn@");
        f::integer_formatter_v<u64_t>.do_format("012X",
                                       (uptr_t)v.as_closure_ref().function(),
                                       to, cs);
        return;
      case tag_rec_ref: to.write("nil"); return;
      case tag_undefined: to.write_styled(st::error, "UNDEF"); return;
      }
    }

    [[nodiscard]] u32_t do_get_length(const s::const_string_view & fo, const lisp_imm & v) const noexcept override {
      switch (v.tag()) {
      case tag_i64:
        return f::integer_formatter_v<i64_t>.do_get_length("", v.as_i64().value());
      case tag_f32:
        return f::float_formatter_v<float>.do_get_length("", v.as_f32().value());

      case tag_bool:
        return v.as_bool().value() ? 4U : 5U;

      case tag_nil:
        return 3U;

      case tag_fn_ref:
        return 15U;
      case tag_closure_ref:
        return 8U + 12U + 3U + 12U;
      case tag_str_ref:
        return 2U + v.as_str_ref()->size();
      case tag_vec_ref: {
        u32_t result = fo == "omit-outer-parens" ? 0U : 2U;
        bool is_first = true;
        for (const lisp_imm & imm : v.as_vec_ref()->value()) {
          if (!is_first) {
            ++result;
          } else {
            is_first = false;
          }
          result += do_get_length("", imm);
        }
        return result;
      }
      case tag_sym_id:
        return e::g_id_formatter.do_get_length("", v.as_sym_id().value());
      case tag_rec_ref: return 3U;
      case tag_undefined: return 5U;
      }
    }
  };

  J_A(ND, NODESTROY) static const lisp_imm_formatter lisp_formatter;

  class lisp_object_formatter final : public s::formatter_known_length<lisp_object, lisp_str, lisp_vec> {
    void do_format(
      const s::const_string_view &,
      const lisp_object & value,
      s::styled_sink & target,
      s::style cs
    ) const override {
      switch (value.type()) {
      case lisp::mem::lowtag_type::str:
        target.write("\"");
        target.write(value.as_str().value());
        target.write("\"");
        return;
      case lisp::mem::lowtag_type::vec: {
        target.write("(");
        bool is_first = true;
        for (const lisp_imm & imm : value.as_vec().value()) {
          if (!is_first) {
            target.write(" ");
          } else {
            is_first = false;
          }
          lisp_formatter.do_format("", imm, target, cs);
        }
        target.write(")");
        return;
      }
      default:
        target.write("[Ctrl]");
      }
    }

    [[nodiscard]] u32_t do_get_length(
      const s::const_string_view &,
      const lisp_object & value
    ) const noexcept override {
      switch (value.type()) {
      case lisp::mem::lowtag_type::str:
        return 2U + value.as_str().size();
      case lisp::mem::lowtag_type::vec: {
        u32_t result = 2U;
        for (const lisp_imm & imm : value.as_vec().value()) {
          if (result) {
            result++;
          }
          result += lisp_formatter.do_get_length("", imm);
        }
        return result + 2U;
      }
      default:
        return 6;
      }
    }
  };

  const s::formatter_known_length<
    lisp_imm,
    lisp_f32, lisp_i64,
    lisp_nil, lisp_bool,
    lisp_fn_ref, lisp_sym_id,
    lisp_str_ref, lisp_vec_ref
  > & g_lisp_imm_formatter = lisp_formatter;

  J_A(ND, NODESTROY) static const lisp_object_formatter object_formatter;
  const s::formatter_known_length<lisp_object, lisp_str, lisp_vec> & g_lisp_object_formatter = object_formatter;
}
