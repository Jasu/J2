#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::cir::inline ops {
  enum class condition : u8_t {
    /// Eq-equality
    eq,
    /// Eq-inequality
    neq,

    /// Unsigned greater than
    ugt,
    /// Unsigned less than
    ult,
    /// Unsigned less than or equal
    ulte,
    /// Unsigned greater than or equal
    ugte,

    /// Signed greater than
    sgt,
    /// Signed less than
    slt,
    /// Signed less than or equal
    slte,
    /// Signed greater than or equal
    sgte,

    /// Floating-point equality
    feq,
    /// Floating-point inequality
    fneq,
    /// Floating-point greater than
    fgt,
    /// Floating-point less than
    flt,
    /// Floating-point less than or equal
    flte,
    /// Floating-point greater than or equal
    fgte,

    /// (A & B) != 0
    band,
    /// (A & B) == 0
    bnand,
    /// (A & 1 << B) != 0
    isset,
    /// (A & 1 << B) == 0
    isclear,
  };

  constexpr condition negate_condition(condition c) noexcept {
    switch (c) {
    case condition::eq:
      return condition::neq;
    case condition::neq:
      return condition::eq;
    case condition::ugt:
      return condition::ulte;
    case condition::ult:
      return condition::ugte;
    case condition::ulte:
      return condition::ugt;
    case condition::ugte:
      return condition::ult;

    case condition::sgt:
      return condition::slte;
    case condition::slt:
      return condition::sgte;
    case condition::slte:
      return condition::sgt;
    case condition::sgte:
      return condition::slt;
    case condition::feq:
      return condition::fneq;
    case condition::fneq:
      return condition::feq;
    case condition::fgt:
      return condition::flte;
    case condition::flt:
      return condition::fgte;
    case condition::flte:
      return condition::fgt;
    case condition::fgte:
      return condition::flt;
    case condition::band:
      return condition::bnand;
    case condition::bnand:
      return condition::band;
    case condition::isset:
      return condition::isclear;
    case condition::isclear:
      return condition::isset;
    }
  }
}
