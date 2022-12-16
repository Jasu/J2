#include "properties/visiting/visit_path.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatting/format_digits.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::properties::formatters {
  namespace {
    namespace s = strings;
    class visit_path_formatter final : public s::formatter_known_length<visiting::visit_path> {
      void do_format(
        const s::const_string_view & format_options,
        const visiting::visit_path & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        if (value.parent()) {
          do_format(format_options, *value.parent(), target, current_style);
        }
        switch (value.type()) {
        case COMPONENT_TYPE::EMPTY:
          return;
        case COMPONENT_TYPE::MAP_KEY: {
          auto & k = value.as_map_key();
          target.write("[\"", 2U);
          target.write(k.data(), k.size());
          target.write("\"]", 2U);
          return;
        }
        case COMPONENT_TYPE::PROPERTY_NAME: {
          if (value.parent() && value.parent()->type() != COMPONENT_TYPE::EMPTY) {
            target.write(".", 1U);
          }
          auto & n = value.as_property_name();
          target.write(n.data(), n.size());
          return;
        }
        case COMPONENT_TYPE::ARRAY_INDEX: {
          target.write("[", 1U);
          const sz_t val = value.as_array_index();
          const u32_t digits = s::num_decimal_digits(val);
          s::formatting::format_dec(val, target.get_write_buffer(digits), digits);
          target.write("]", 1U);
          return;
        }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const visiting::visit_path & value
      ) const noexcept override {
        const visiting::visit_path * p = &value;
        u32_t result = 0;
        while (p) {
          switch (p->type()) {
          case COMPONENT_TYPE::EMPTY:
            break;
          case COMPONENT_TYPE::MAP_KEY:
            result += 4U + p->as_map_key().size();
            break;
          case COMPONENT_TYPE::PROPERTY_NAME:
            if (p->parent() && p->parent()->type() != COMPONENT_TYPE::EMPTY) {
              result++;
            }
            result += p->as_property_name().size();
            break;
          case COMPONENT_TYPE::ARRAY_INDEX:
            result += s::num_decimal_digits(p->as_array_index()) + 2U;
            break;
          }
          p = p->parent();
        }
        return result;
      }
    };
    J_A(ND, NODESTROY) const visit_path_formatter g_visit_path_formatter;
  }
}
