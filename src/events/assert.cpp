#include "events/assert.hpp"

#include "exceptions/exceptions.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/styling/default_styles.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    namespace s = strings;

    class J_TYPE_HIDDEN uv_error_formatter final : public s::formatter_known_length<int> {
    public:
      uv_error_formatter()
        : formatter_known_length<int>("libuv-error", false)
      { }

      void do_format(
        const s::const_string_view &,
        const int & value,
        s::styled_sink & target,
        s::style
      ) const override {
        target.write_styled(s::styles::bold_bright_red, uv_err_name(value));
        target.write(" ");
        target.write_styled(s::styles::bright_red, uv_strerror(value));
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const int & value
      ) const noexcept override {
        return j::strlen(uv_err_name(value)) + 1U + j::strlen(uv_strerror(value));
      }
    };

    J_A(ND, NODESTROY) const uv_error_formatter g_uv_error_formatter;
  }

  [[noreturn]] void fail_function(const char *name, int error) {
    J_THROW(exception() << service_name("UV") << function_name(name) << uv_error(error));
  }

  const tags::tag_definition<int> uv_error{"libuv error", "{libuv-error}"};
}
