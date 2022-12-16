#include "lisp/air/functions/air_closure.hpp"
#include "lisp/air/debug/debug_formatters.hpp"
#include "lisp/env/debug_formatters.hpp"
#include "lisp/air/functions/activation_record.hpp"
#include "lisp/symbols/symbol.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::air {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    class J_TYPE_HIDDEN lambda_info_formatter final : public s::formatter_known_length<air_closure> {
    public:
      void do_format(
        const s::const_string_view &,
        const air_closure & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        to.write_styled(s::styles::bright_green, "Lambda (scope depth=");
        f::integer_formatter_v<i32_t>.do_format("", v.scope_depth, to, cs);
        to.write_styled(s::styles::bright_green, ")");
        u32_t i = 0U;
        for (auto & scope : v.scopes()) {
          to.write("\n  Scope #");
          f::integer_formatter_v<i32_t>.do_format("", i, to, cs);
          to.write(": ");
          g_expr_type_formatter.do_format("", scope.expr->type, to, cs);
          bool is_first = true;
          for (auto & v : scope) {
            to.write(is_first ? " Vars: " : ", ");
            env::g_id_formatter.do_format("", v.name, to, cs);
            is_first = false;
          }
          ++i;
        }
        to.write("\n");
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const air_closure & v
      ) const noexcept override {
        u32_t result = 20U
         + f::integer_formatter_v<i32_t>.do_get_length("", v.scope_depth)
          + 1U;
        u32_t i = 0U;
        for (auto & scope : v.scopes()) {
          result += 10U
            + f::integer_formatter_v<i32_t>.do_get_length("", i)
            + 2U
            + g_expr_type_formatter.do_get_length("", scope.expr->type);
            bool is_first = true;
            for (auto & v : scope) {
              result += is_first ? 7U : 2U;
              result += env::g_id_formatter.do_get_length("", v.name);
              is_first = false;
            }
          ++i;
        }
        return 1U + result;
      }
    };
    J_A(ND, NODESTROY) const lambda_info_formatter info_fmt;

    class J_TYPE_HIDDEN activation_record_formatter final : public s::formatter_known_length<activation_record> {
    public:
      void do_format(
        const s::const_string_view &,
        const activation_record & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        to.write_styled(s::styles::bright_yellow, "Activation record #");
        f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.index, to, cs);
        to.write(":  ");
        f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.num_closures, to, cs);
        to.write(" closures,  ");
        f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.num_vars, to, cs);
        to.write(v.parent ? " vars, parent: #" : " vars, no parent.");
        if (v.parent) {
          f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.parent->index, to, cs);
        }
        to.write("\n");
        to.write_styled(s::styles::bright_cyan, "   Vars: ");
        bool is_first = true;
        for (auto & var : v.vars()) {
          if (!is_first) {
            to.write(", ");
          }
          is_first = false;
          env::g_id_formatter.do_format("", var->name, to, cs);
        }
        to.write("\n  Lowest expr:\n  ");
        g_expr_formatter.do_format("", *v.lowest_scope, to, cs);

        // to.write_styled(s::styles::bright_magenta, "   Closures: ");
        // bool is_first = true;
        // for (auto & c : v.closures()) {
        //   if (!is_first) {
        //     to.write(", ");
        //   }
        //   is_first = false;
        //   to.write("{}", c->name);
        // }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const activation_record & v
      ) const noexcept override {
        u32_t result = 19U
          + f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.index)
          + 2U
          + f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.num_closures)
          + 12U
          + f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.num_vars)
        + 16U
          + (v.parent
             ? f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.parent->index)
             : 0)
          + 1U + 9U;
        bool is_first = true;
        for (auto & var : v.vars()) {
          if (!is_first) {
            result += 2U;
          }
          is_first = false;
          result += env::g_id_formatter.do_get_length("", var->name);
        }
        result += 17U + g_expr_formatter.do_get_length("", *v.lowest_scope);
        return 1U + result;
      }
    };
    J_A(ND, NODESTROY) const activation_record_formatter act_fmt;
  }
}
