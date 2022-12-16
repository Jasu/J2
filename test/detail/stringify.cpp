#include "stringify.hpp"

#include "strings/format.hpp"
#include "strings/unicode/utf8.hpp"
#include "strings/unicode/utf8_string_view.hpp"
#include "strings/unicode/line_breaks.hpp"
#include "ui/layout/size.hpp"
#include "lisp/values/lisp_str.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "lisp/air/exprs/expr.hpp"

namespace j::strings {
    J_A(AI,ND,NODISC) static inline doctest::String toStringNoEsc(const string & s) {
      return { s.data(), (unsigned int)s.size() };
    }

    J_A(ND) doctest::String toString(const const_string_view & sv) {
      string s(format("{:$u}", sv));
      return { s.data(), (unsigned int)s.size() };
    }

    J_A(ND) doctest::String toString(const string_view & s) {
      return toString((const_string_view)s);
    }

    J_A(ND) doctest::String toString(const string & s) {
      return toString((const_string_view)s);
    }
}
namespace j::strings::inline unicode {
  J_A(ND) doctest::String toString(const const_utf8_string_view & s) {
    return toString((const_string_view)s);
  }
  J_A(ND) doctest::String toString(const utf8_string_view & s) {
    return toString((const_string_view)s);
  }
  J_A(ND) doctest::String toString(const lbr_state & s) {
    return toStringNoEsc(j::strings::format("{}", s));
  }
  J_A(ND) doctest::String toString(const line_break_state & s) {
    return toStringNoEsc(j::strings::format("{}", s));
  }
}

namespace j::ui::inline layout {
  J_A(ND) doctest::String toString(const size & v) {
    return j::strings::toStringNoEsc(j::strings::format("{}", v));
  }
  J_A(ND) doctest::String toString(const span & v) {
    return j::strings::toStringNoEsc(j::strings::format("{}", v));
  }
}

namespace j::lisp::air {
  J_A(ND) doctest::String toString(const expr_type & v) {
    return j::strings::toStringNoEsc(j::strings::format("{}", v));
  }
}

namespace j::lisp::air::exprs {
  J_A(ND) doctest::String toString(const struct expr & v) {
    return j::strings::toStringNoEsc(j::strings::format("{}", v));
  }
}

namespace j::lisp::inline values {
  J_A(ND) doctest::String toString(const lisp_str & v) {
    return j::strings::toString(j::strings::const_string_view{ v.data(), v.size() });
  }
  J_A(ND) doctest::String toString(const lisp_imm & v) {
    return j::strings::toStringNoEsc(j::strings::format("{}", v));
  }
}
