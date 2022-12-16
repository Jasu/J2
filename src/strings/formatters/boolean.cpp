#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/string_view.hpp"
#include "strings/string_algo.hpp"

namespace j::strings::formatters {
  namespace {
    class bool_formatter final : public formatter_known_length<bool> {
    public:
      void do_format(
        const const_string_view & format_options,
        const bool & value,
        styled_sink & target,
        style
      ) const override {
        if (!format_options) {
          target.write(value ? "true" : "false", value ? 4U : 5U);
        } else {
          const u32_t comma = find_first(format_options, ',');
          J_ASSERT(comma >= 0, "Comma not found.");
          target.write(format_options.begin() + (value ? 0U : comma + 1U),
                       value ? comma : format_options.size() - comma - 1U);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view & format_options,
        const bool & value
      ) const noexcept override {
        if (!format_options) {
          return value ? 4U : 5U;
        } else {
          const u32_t comma = find_first(format_options, ',');
          J_ASSERT(comma >= 0, "Comma not found.");
          return value ? comma : format_options.size() - comma - 1U;
        }
      }
    };
    J_A(ND, NODESTROY) const bool_formatter g_bool_formatter;
  }
}
