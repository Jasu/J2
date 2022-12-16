#include "strings/unicode/rope/rope_utf8_metrics.hpp"
#include "strings/formatters/pointers.hpp"
#include "strings/unicode/rope/rope_utf8_value.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/integers.hpp"

namespace j::strings::inline unicode::inline rope {
  namespace {
    namespace s = strings;
    namespace f = formatters;

    class J_TYPE_HIDDEN rope_utf8_metrics_formatter final : public formatter_known_length<rope_utf8_metrics> {
    public:
      void do_format(const const_string_view &, const rope_utf8_metrics & v, styled_sink & to, style cs) const override {
        f::integer_formatter_v<u32_t>.do_format("", v.size_bytes, to, cs);
        to.write("B, ");
        f::integer_formatter_v<u32_t>.do_format("", v.num_hard_breaks, to, cs);
        to.write("HB");
      }

      u32_t do_get_length(const const_string_view &, const rope_utf8_metrics & v) const noexcept override {
        return f::integer_formatter_v<u32_t>.do_get_length("", v.size_bytes) + 3U
          + f::integer_formatter_v<u32_t>.do_get_length("", v.num_hard_breaks) + 2U;
      }
    };

    J_A(ND, NODESTROY) const rope_utf8_metrics_formatter g_utf8_rope_metrics_fmt;

    class J_TYPE_HIDDEN rope_utf8_value_formatter final : public formatter_known_length<rope_utf8_value> {
  public:
    void do_format(const const_string_view &, const rope_utf8_value & v, styled_sink & to, style cs) const override {
      u32_t before = v.text.begin() - v.m_string->begin();
      u32_t after = v.m_string->end() - v.text.end();
      to.write("|-");
      f::integer_formatter_v<u32_t>.do_format("", before, to, cs);
      to.write("-|-");
      f::integer_formatter_v<u32_t>.do_format("", v.text.size(), to, cs);
      to.write("-|-");
      f::integer_formatter_v<u32_t>.do_format("", after, to, cs);
      to.write("-| ");
      f::g_void_star_formatter.do_format("", v.text.begin(), to, cs);
    }

    u32_t do_get_length(const const_string_view &, const rope_utf8_value & v) const noexcept override {
      u32_t before = v.text.begin() - v.m_string->begin();
      u32_t after = v.m_string->begin() - v.text.end();
      return 2U + f::integer_formatter_v<u32_t>.do_get_length("", before)
        + 3U + f::integer_formatter_v<u32_t>.do_get_length("", v.text.size())
        + 3U + f::integer_formatter_v<u32_t>.do_get_length("", after)
        + 3U + f::g_void_star_formatter.do_get_length("", v.text.begin());
      }
    };

    J_A(ND, NODESTROY) const rope_utf8_value_formatter g_utf8_rope_value_fmt;
  }
}
