#pragma once

#include "hzd/utility.hpp"

namespace j::strings {
  template<typename> class basic_string_view;
  using const_string_view = basic_string_view<const char>;
}
namespace j::strings::inline styling { class styled_sink; }
namespace j::strings::inline formatting { struct escape_options; }

namespace j::strings::formatters {
  enum class truncate_mode_t : u8_t {
    right = 0U,
    left,
    center,
  };

  struct string_format_options final {
    u8_t pad_left_by = 0U;
    u8_t pad_right_by = 0U;
    u8_t pad_to = 0U;
    u8_t truncate_to = 0U;
    truncate_mode_t truncate_mode = truncate_mode_t::right;
    bool right_align = false;
    const formatting::escape_options * escape_options = nullptr;
  };

  string_format_options parse_string_format_options(const_string_view opts);

  i32_t escaped_size(const string_format_options & opts, const_string_view str) noexcept;

  i32_t write_escaped(styled_sink & target, const string_format_options & opts, const_string_view str, i32_t sz, bool is_left);

  i32_t compute_string_size(const string_format_options & opts, i32_t length);

  void write_string_padding(styled_sink & target, string_format_options opts, u32_t length, bool is_right);

  i32_t compute_string_left_size(const string_format_options & opts, i32_t length) noexcept;

  i32_t compute_string_right_size(const string_format_options & opts, i32_t length) noexcept;

  void write_string_ellipsis(styled_sink & target, string_format_options opts, u32_t length);
}
