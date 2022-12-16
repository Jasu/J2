#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::assembly {
  constexpr inline u8_t z_flag  = 0b00001U;
  constexpr inline u8_t nz_flag = 0b00010U;
  constexpr inline u8_t s_flag  = 0b00100U;
  constexpr inline u8_t ns_flag = 0b01000U;
  constexpr inline u8_t c_flag  = 0b00100U;
  constexpr inline u8_t nc_flag = 0b01000U;
  constexpr inline u8_t or_flag = 0b10000U;

  enum class test_flag_condition : u8_t {
    z         = z_flag,
    nz        = nz_flag,
    s         = s_flag,
    ns        = ns_flag,
    s_and_z   = s_flag  | z_flag,
    s_and_nz  = s_flag  | nz_flag,
    ns_and_z  = ns_flag | z_flag,
    ns_and_nz = ns_flag | nz_flag,
    s_or_z    = or_flag | s_flag  | z_flag,
    s_or_nz   = or_flag | s_flag  | nz_flag,
    ns_or_z   = or_flag | ns_flag | z_flag,
    ns_or_nz  = or_flag | ns_flag | nz_flag,
  };

  enum class cmp_flag_condition : u8_t {
    eq   = 0b11000,
    neq  = 0b11111,

    ult  = 0b10011,
    ulte = 0b10010,
    ugt  = 0b10101,
    ugte = 0b10100,

    slt  = 0b01011,
    slte = 0b01010,
    sgt  = 0b01101,
    sgte = 0b01100,
  };

  constexpr cmp_flag_condition negate_condition(cmp_flag_condition c) {
    return (cmp_flag_condition)((u8_t)c ^ 0b0111);
  }

  constexpr cmp_flag_condition reverse_condition(cmp_flag_condition c) {
    return (c == cmp_flag_condition::eq ||c == cmp_flag_condition::neq) ? c : (cmp_flag_condition)((u8_t)c ^ 0b0111);
  }
  constexpr test_flag_condition negate_condition(test_flag_condition c) {
    switch (c) {
    case test_flag_condition::z:         return test_flag_condition::nz;
    case test_flag_condition::nz:        return test_flag_condition::z;
    case test_flag_condition::s:         return test_flag_condition::ns;
    case test_flag_condition::ns:        return test_flag_condition::s;
    case test_flag_condition::s_and_z:   return test_flag_condition::ns_or_nz;
    case test_flag_condition::s_and_nz:  return test_flag_condition::ns_or_z;
    case test_flag_condition::ns_and_z:  return test_flag_condition::s_or_nz;
    case test_flag_condition::ns_and_nz: return test_flag_condition::s_or_z;
    case test_flag_condition::s_or_z:    return test_flag_condition::ns_and_nz;
    case test_flag_condition::s_or_nz:   return test_flag_condition::ns_and_z;
    case test_flag_condition::ns_or_z:   return test_flag_condition::s_and_nz;
    case test_flag_condition::ns_or_nz:  return test_flag_condition::s_and_z;
    }
  }
}
