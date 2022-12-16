#include "strings/formatting/formatter.hpp"
#include "strings/formatters/strings.hpp"
#include "files/paths/path.hpp"

namespace j::files {
  namespace s = j::strings;
  namespace {
    class J_TYPE_HIDDEN path_formatter final : public s::formatter_known_length<path> {
    public:
      void do_format(
        const s::const_string_view & format_options,
        const path & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        s::formatters::g_string_formatter.do_format(
          format_options, (const strings::string &)value,
          target, current_style);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & format_options,
        const path & value) const noexcept override {
        return s::formatters::g_string_formatter.do_get_length(
          format_options, (const strings::string &)value);
      }
    };

    J_A(ND, NODESTROY) const path_formatter g_path_formratter;
  }
}
