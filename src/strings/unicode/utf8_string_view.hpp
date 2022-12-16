#pragma once

#include "strings/string_view.hpp"
#include "strings/unicode/utf8.hpp"

namespace j::strings::inline unicode {
  struct basic_utf8_string_view_lbrs;
  struct basic_utf8_string_view_code_points;

  template<typename Char>
  class basic_utf8_string_view : public basic_string_view<Char>
  {
  public:
    using basic_string_view<Char>::front;
    using basic_string_view<Char>::empty;
    using basic_string_view<Char>::size;
    using basic_string_view<Char>::basic_string_view;

    using basic_string_view<Char>::operator[];

    J_INLINE_GETTER basic_utf8_string_view suffix(u32_t len) const noexcept
    { return basic_string_view<Char>::suffix(len); }

    J_INLINE_GETTER basic_utf8_string_view prefix(u32_t len) const noexcept
    { return basic_string_view<Char>::prefix(len); }

    J_INLINE_GETTER basic_utf8_string_view without_suffix(u32_t len) const noexcept
    { return basic_string_view<Char>::without_suffix(len); }

    J_INLINE_GETTER basic_utf8_string_view without_prefix(u32_t len) const noexcept
    { return basic_string_view<Char>::without_prefix(len); }

    J_INLINE_GETTER basic_utf8_string_view take_prefix(u32_t len) noexcept
    { return basic_string_view<Char>::take_prefix(len); }

    J_INLINE_GETTER basic_utf8_string_view take_suffix(u32_t len) noexcept
    { return basic_string_view<Char>::take_suffix(len); }

    J_ALWAYS_INLINE basic_utf8_string_view & remove_prefix(u32_t len) noexcept {
      basic_string_view<Char>::remove_prefix(len);
      return *this;
    }

    J_INLINE_GETTER bool starts_with_successor_byte() const noexcept {
      return !empty() && utf8_is_successor_byte(front());
    }

    [[nodiscard]] u8_t truncated_by() const noexcept {
      u32_t sz = size();
      if (!sz--) { return 0U; }
      if (!utf8_is_successor_byte(operator[](sz))) {
        return utf8_leading_byte_code_point_bytes(operator[](sz)) - 1U;
      }
      if (!sz--) { return 0U; }
      if (!utf8_is_successor_byte(operator[](sz))) {
        return utf8_leading_byte_code_point_bytes(operator[](sz)) - 2U;
      }
      if (!sz--) { return 0U; }
      if (!utf8_is_successor_byte(operator[](sz))) {
        return utf8_leading_byte_code_point_bytes(operator[](sz)) - 3U;
      }
      return 0U;
    }

    J_ALWAYS_INLINE basic_utf8_string_view & remove_suffix(u32_t len) noexcept {
      basic_string_view<Char>::remove_suffix(len);
      return *this;
    }


    J_INLINE_GETTER basic_utf8_string_view slice(u32_t start, u32_t len) const noexcept
    { return basic_string_view<Char>::slice(start, len); }

    constexpr basic_utf8_string_view() noexcept = default;

    template<typename OtherChar>
    basic_utf8_string_view(const basic_string_view<OtherChar> & rhs) noexcept
      : basic_string_view<Char>(rhs)
    { }

    J_INLINE_GETTER operator const basic_utf8_string_view<const char> &() const noexcept
    { return *reinterpret_cast<const basic_utf8_string_view<const char> *>(this); }

    /// Iterate the string view as line breaks.
    J_INLINE_GETTER const basic_utf8_string_view_lbrs & line_breaks() const noexcept
    { return *reinterpret_cast<const basic_utf8_string_view_lbrs *>(this); }

    /// Iterate the string view as code points.
    J_INLINE_GETTER const basic_utf8_string_view_code_points & code_points() const noexcept
    { return *reinterpret_cast<const basic_utf8_string_view_code_points *>(this); }

    friend struct basic_utf8_string_view_lbrs;
    friend struct basic_utf8_string_view_code_points;
  };

  using utf8_string_view = basic_utf8_string_view<char>;
  using const_utf8_string_view = basic_utf8_string_view<const char>;
}
