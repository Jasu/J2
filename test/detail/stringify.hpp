#pragma once

#include <doctest_fwd.h>
#include "hzd/utility.hpp"

namespace j::strings {
  class string;
  template<typename> class basic_string_view;
  using string_view = basic_string_view<char>;
  using const_string_view = basic_string_view<const char>;

  J_A(ND) doctest::String toString(const const_string_view & sv);
  J_A(ND) doctest::String toString(const string_view & s);
  J_A(ND) doctest::String toString(const string & s);

  inline namespace unicode {
    template<typename> class basic_utf8_string_view;
    using utf8_string_view = basic_utf8_string_view<char>;
    using const_utf8_string_view = basic_utf8_string_view<const char>;
    J_A(ND) doctest::String toString(const const_utf8_string_view & s);
    doctest::String toString(const utf8_string_view & s);
    enum class lbr_state : u8_t;
    J_A(ND) doctest::String toString(const lbr_state & s);
    struct line_break_state;
    J_A(ND) doctest::String toString(const line_break_state & s);
  }
}

namespace j::ui::inline layout {
  struct span;
  J_A(ND) doctest::String toString(const struct size & v);
  J_A(ND) doctest::String toString(const span & v);
}
namespace j::lisp::air {
  enum class expr_type : u8_t;
  J_A(ND) doctest::String toString(const expr_type & v);
}
namespace j::lisp::air::exprs {
  J_A(ND) doctest::String toString(const struct expr & v);
}

namespace j::lisp::inline values {
  J_A(ND) doctest::String toString(const struct lisp_str & v);
  J_A(ND) doctest::String toString(const struct lisp_imm & v);
}
