#pragma once

#include "strings/string_view.hpp"
#include "lisp/common/id.hpp"

namespace j::strings {
  class string;
  inline namespace styling {
    class styled_sink;
    struct style;
  }
}

namespace j::lisp::inline common {
  enum class operand_name_format : u8_t {
    index_bracket,
    index_offset,

    default_format = index_offset,
  };

  struct operand_name final {
    id id{};
    i32_t index:28 = 0;
    operand_name_format name_format:4 = operand_name_format::default_format;
    strings::const_string_view name{};

    strings::string format() const noexcept;
    void format(strings::styling::styled_sink & to, strings::styling::style cs) const noexcept;
    u32_t get_length() const noexcept;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return id || name;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !id && !name;
    }
  };
}
