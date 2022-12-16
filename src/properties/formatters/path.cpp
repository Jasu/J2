#include "properties/path.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatting/format_digits.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::properties::stringifiers {
  namespace {
    namespace s = strings;
    class path_formatter final : public s::formatter_known_length<path> {
      void do_format(
        const s::const_string_view &,
        const path & value,
        s::styled_sink & target,
        s::style
      ) const override {
        const u32_t num_components = value.size();
        for (u32_t i = 0; i < num_components; ++i) {
          switch (value[i].type()) {
          case COMPONENT_TYPE::EMPTY:
            target.write(i == 0 ? "*EMPTY-ERROR*" : ".*EMPTY-ERROR*", i == 0 ? 13U : 14U);
            break;
          case COMPONENT_TYPE::MAP_KEY:
            target.write("[\"", 2U);
            {
              auto & k = value[i].as_map_key();
              target.write(k.data(), k.size());
            }
            target.write("\"]", 2U);
            break;
          case COMPONENT_TYPE::PROPERTY_NAME:
            if (i != 0) {
              target.write(".", 1);
            }
            {
              auto & k = value[i].as_property_name();
              target.write(k.data(), k.size());
            }
            break;
          case COMPONENT_TYPE::ARRAY_INDEX:
            target.write("[", 1U);
            {
              const sz_t val = value[i].as_array_index();
              const int digits = s::num_decimal_digits(val);
              s::formatting::format_dec(val, target.get_write_buffer(digits), digits);
            }
            target.write("]", 1U);
            break;
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const path & value
      ) const noexcept override {
        u32_t sz = 0;
        const u32_t num_components = value.size();
        for (u32_t i = 0U; i < num_components; ++i) {
          switch (value[i].type()) {
          case COMPONENT_TYPE::EMPTY:
            sz += (i == 0) ? 13 : 14;
            break;
          case COMPONENT_TYPE::MAP_KEY:
            sz += 4 + value[i].as_map_key().size();
            break;
          case COMPONENT_TYPE::PROPERTY_NAME:
            sz += (i == 0 ? 0 : 1) + value[i].as_property_name().size();
            break;
          case COMPONENT_TYPE::ARRAY_INDEX:
            sz += 2 + s::num_decimal_digits(value[i].as_array_index());
            break;
          }
        }
        return sz;
      }
    };

    J_A(ND, NODESTROY) const path_formatter g_path_formatter;
  }
}
