#include "lisp/sources/debug_formatters.hpp"

#include "lisp/sources/source.hpp"
#include "lisp/sources/source_manager.hpp"
#include "lisp/sources/source_location.hpp"
#include "lisp/env/context.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/env_compiler.hpp"

#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::lisp::sources {
  namespace s = strings;
  namespace f = s::formatters;

  namespace {
    class J_TYPE_HIDDEN source_location_formatter final : public s::formatter_known_length<source_location> {
    public:
      void do_format(
        const s::const_string_view &,
        const source_location & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        if (!v.is_valid) {
          to.write_styled(s::styles::bright_red, "Invalid src loc");
          return;
        }
        auto * env = env::env_context->env;
        auto cmp = env ? env->maybe_get_env_compiler() : nullptr;
        if (cmp && cmp->source_manager.sources.size() > v.source_index) {
          to.write_styled(s::styles::bright_green, cmp->source_manager.sources[v.source_index]->name_str());
        } else {
          to.set_style(s::styles::bright_red);
          f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.source_index, to, s::styles::bright_red);
        }
        to.write_styled(s::styles::light_gray, ":");
        to.set_style(s::styles::cyan);
        f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.line, to, s::styles::cyan);
        to.write_styled(s::styles::gray, ":");
        to.set_style(s::styles::cyan);
        f::integer_formatter_v<u32_t>.do_format("", (u32_t)v.column, to, s::styles::cyan);
        to.set_style(cs);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const source_location & v
      ) const noexcept override {
        if (!v.is_valid) {
          return 15U;
        }
        u32_t result = 0U;
        auto * env = env::env_context->env;
        auto cmp = env ? env->maybe_get_env_compiler() : nullptr;
        if (cmp && cmp->source_manager.sources.size() > v.source_index) {
          result = cmp->source_manager.sources[v.source_index]->name_str().size();
        } else {
          result = f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.source_index);
        }
        return result + 1U
          + f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.line)
          + 1U + f::integer_formatter_v<u32_t>.do_get_length("", (u32_t)v.column);
      }
    };

    J_A(ND, NODESTROY) const source_location_formatter source_loc_fmt;
  }
  const strings::formatter_known_length<source_location> & g_source_location_formatter = source_loc_fmt;
}
