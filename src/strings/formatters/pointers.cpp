#include "strings/formatters/pointers.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::strings::formatters {
  namespace {
    class void_star_formatter final : public formatter_known_length<const void *, void *> {
    public:
      void do_format(
        const const_string_view &,
        const void * const & value,
        styled_sink & out,
        style cs
      ) const override {
        if (!value) {
          out.write_styled(styles::bright_red, "(null)");
          return;
        }
        out.write_styled(
          (cs.foreground.empty() && cs.background.empty()) ? styles::bright_green : cs,
          "0x");
        integer_formatter_v<u64_t>.do_format(((uptr_t)value & 0xFFFF000000000000ULL)
                                    ? "016X" : "012X",
                                    (u64_t)value,
                                    out,
                                    cs);
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view &,
        const void * const & value) const noexcept override
      {
        if (!value) {
          return 6U; // (null)
        }
        if ((uptr_t)value & 0xFFFF000000000000ULL) {
          return 2U + 16U;
        }
        return 2U + 12U;
      }
    };
    J_A(ND, NODESTROY) const void_star_formatter g_void_star_fmt;
  }
  constinit const formatter_known_length<const void*, void*> & g_void_star_formatter = g_void_star_fmt;
}
