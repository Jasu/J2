#include "errors.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "files/paths/path.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::meta {
  const tags::tag_definition<source_location> source_loc{"{#bright_magenta,bold}At #{}{/}", "{}", tags::tag_name_format::with_index};
  namespace {
    namespace st = strings::styles;
    namespace s = strings;
    namespace f = strings::formatters;

    class J_TYPE_HIDDEN source_location_formatter final : public s::formatter_known_length<source_location> {
    public:
      void do_format(
        const s::const_string_view &,
        const source_location & v,
        s::styled_sink & to,
        s::style cs
      ) const override {
        if (v.cur_file) {
          to.write_styled(st::bold, v.cur_file->basename());
          if (v.line >= 0) {
            to.write(":");
          }
        }
        if (v.line >= 0) {
          f::integer_formatter_v<i32_t>.do_format("", v.line, to, cs);
          to.write(":");
          f::integer_formatter_v<i32_t>.do_format("", v.column, to, cs);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const source_location & v
      ) const noexcept override {
        u32_t result = 0;
        if (v.cur_file) {
          result += v.cur_file->basename().size();
          if (v.line >= 0) {
            ++result;
          }
        }
        if (v.line >= 0) {
          result += f::integer_formatter_v<i32_t>.do_get_length("", v.line)
            + 1 + f::integer_formatter_v<i32_t>.do_get_length("", v.column);
        }
        return result;
      }
    };

    J_A(ND,NODESTROY) const source_location_formatter g_loc_fmt;
  }

  error::error(const char * J_NOT_NULL msg, source_location loc, err_severity sev) noexcept
    : msg(msg), loc(loc), sev(sev)
  { }

  error::error(strings::string && msg, source_location loc, err_severity sev) noexcept
    : msg(static_cast<strings::string &&>(msg)), loc(loc), sev(sev)
  { }
}
