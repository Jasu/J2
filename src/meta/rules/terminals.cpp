#include "terminals.hpp"
#include "meta/term.hpp"
#include "meta/module.hpp"
#include "exceptions/assert.hpp"

namespace s = j::strings;

namespace j::meta::inline rules {
  u8_t terminal_collection::add(strings::const_string_view name, u8_t min_operands, u8_t max_operands) noexcept {
    J_REQUIRE(size < 255, "Terminal overflow");
    J_ASSERT(min_operands <= max_operands);
    defs[size] = { name, min_operands, max_operands };
    any_terminal += terminal{size};
    any_terminal_or_exit += terminal{size};
    return size++;
  }

  [[nodiscard]] s::const_string_view terminal_collection::name_of(module * mod, terminal t) const noexcept {
    if (t.index == 255) {
      return "𝜺";
    } else if (t.index == 254) {
      return "exit";
    }
    if (mod) {
      const term * term = mod->terms().at(t.index);
      J_ASSERT_NOT_NULL(term);
      if (const s::string * name = term->attrs.maybe_string_at("ShortName")) {
        return *name;
      }
      return term->name;
    }
    return defs[t.index].name;
  }
}
