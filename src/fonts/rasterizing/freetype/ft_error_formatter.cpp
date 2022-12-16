#include "fonts/rasterizing/freetype/assert.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_sink.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace j::fonts::rasterizing::freetype {
  namespace {
    struct J_TYPE_HIDDEN ft_error_def {
      FT_Error code;
      const char * description;
    };

#define FT_NOERRORDEF_(_, CODE, DESC) { (CODE), (DESC) },
#define FT_ERRORDEF_(_, CODE, DESC) { (CODE), (DESC) },
    constexpr ft_error_def g_errors[]{
#include FT_ERROR_DEFINITIONS_H
    };
#undef FT_NOERRORDEF_
#undef FT_ERRORDEF_

    namespace s = strings;
    class J_TYPE_HIDDEN ft_error_formatter final : public s::formatter_known_length<FT_Error> {
    public:
      ft_error_formatter()
        : formatter_known_length<FT_Error>("freetype-error", false)
      { }

      void do_format(
        const s::const_string_view &,
        const FT_Error & value,
        s::styled_sink & target,
        s::style
      ) const override {
        for (u32_t i = 0U; i < J_ARRAY_SIZE(g_errors); ++i) {
          if (g_errors[i].code == value) {
            target.write(g_errors[i].description);
            return;
          }
        }
        target.write("Unknown error");
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const FT_Error & value
      ) const noexcept override {
        u32_t len = ::j::strlen("Unknown error");
        for (u32_t i = 0U; i < J_ARRAY_SIZE(g_errors); ++i) {
          if (g_errors[i].code == value) {
            len = ::j::strlen(g_errors[i].description);
          }
        }
        return len;
      }
    };

    J_A(ND, NODESTROY) const ft_error_formatter g_ft_error_formatter;
  }

  const tags::tag_definition<FT_Error> ft_error{"Freetype2 Error", "{freetype-error}"};

  [[noreturn]] void fail_function(const char * const name, FT_Error error) {
    J_THROW(exception() << service_name("Freetype") << function_name(name) << ft_error(error));
  }
}
