#pragma once

#include "strings/unicode/utf8_string_view.hpp"
#include "strings/unicode/utf8_lbr_iterator.hpp"

namespace j::strings::inline unicode {
  struct basic_utf8_string_view_lbrs {
    utf8_lbr_iterator begin() const noexcept {
      auto & sv = (const const_string_view &)*reinterpret_cast<const const_utf8_string_view *>(this);
      return { sv.begin(), sv.end() };
    }
    utf8_lbr_iterator end() const noexcept {
      auto & sv = (const const_string_view &)*reinterpret_cast<const const_utf8_string_view *>(this);
      return { sv.end(), sv.end() };
    }
  };

  struct basic_utf8_string_view_code_points {
    utf8_code_point_iterator begin() const noexcept {
      auto & sv = (const const_string_view &)*reinterpret_cast<const const_utf8_string_view *>(this);
      return { sv.begin(), sv.end() };
    }
    utf8_code_point_iterator end() const noexcept {
      auto & sv = (const const_string_view &)*reinterpret_cast<const const_utf8_string_view *>(this);
      return { sv.end(), sv.end() };
    }
  };

}
