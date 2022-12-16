#include "lisp/env/debug_formatters.hpp"
#include "lisp/env/context.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/packages/pkg.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/default_styles.hpp"


namespace j::lisp::env {
  namespace s = strings;
  namespace f = s::formatters;

  namespace {
    class id_formatter final : public s::formatter_known_length<id> {
    public:
      void do_format(
        const s::const_string_view & fo,
        const id & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        if (!v) {
          to.write_styled(s::styles::bright_red, "[EmptyName]");
          return;
        }

        u8_t package_id = v.package_id();
        if (v.is_gensym()) {
          to.write_styled(s::styles::bright_yellow, "%");
        } else {
          if (auto * env = env_context->env) {
            if (v.is_unresolved()) {
              auto pair = env->package_at(v.package_id())->symbol_table.names.get_unresolved_names(v);
              to.write_styled(s::styles::red, pair.pkg_name->value());
              to.write_styled(s::styles::bold, ":");
              to.write_styled(s::styles::bright_red, pair.sym_name->value());
            }  else {
              auto * current_pkg = env_context->current_pkg;
              bool is_kwd = package_id == package_id_keyword;
              if (is_kwd) {
                to.write_styled(s::styles::bright_yellow, ":");
              } else if (package_id != package_id_global && (!current_pkg || current_pkg->symbol_table.names.package_id != package_id)) {
                to.write_styled(s::styles::cyan, env->package_name(v.package_id()).value());
                to.write_styled(s::styles::bold, ":");
              }
              to.write_styled(is_kwd ? s::styles::bright_yellow : s::styles::bright_cyan,
                              env->symbol_name(v)->value());
            }
            if (fo != "with-id") {
              return;
            }
          }
        }

        to.write_styled(s::styles::bold, "#");
        to.set_style(s::styles::light_gray);
        f::integer_formatter_v<u8_t>.do_format("", v.package_id(), to, s::styles::light_gray);
        to.write_styled(s::styles::bold, ":");
        f::integer_formatter_v<u32_t>.do_format("", v.index(), to, s::styles::light_gray);
        to.set_style(cs);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & fo,
        const id & v
      ) const noexcept override {
        if (!v) {
          return 11;
        }

        u32_t result = 0U;
        u8_t package_id = v.package_id();
        if (v.is_gensym()) {
          result = 1U;
        } else {
          if (auto * env = env_context->env) {
            if (v.is_unresolved()) {
              auto pair = env->package_at(v.package_id())->symbol_table.names.get_unresolved_names(v);
              result += pair.pkg_name->size() + 1U + pair.sym_name->size();
            } else {
              auto * current_pkg = env_context->current_pkg;
              if (package_id == package_id_keyword) {
                result++;
              } else if (package_id != package_id_global && (!current_pkg || current_pkg->symbol_table.names.package_id != package_id)) {
                result += 1 + env->package_name(v.package_id()).size();
              }
              result += env->symbol_name(v)->size();
            }
            if (fo != "with-id") {
              return result;
            }
          }
        }
        return result + 1U + f::integer_formatter_v<u8_t>.do_get_length("", v.package_id()) + 1 + f::integer_formatter_v<u32_t>.do_get_length("", v.index());
      }
    };

    J_A(ND, NODESTROY) const id_formatter g_fmt;
  }

  constinit const strings::formatter_known_length<id> & g_id_formatter = g_fmt;
}
