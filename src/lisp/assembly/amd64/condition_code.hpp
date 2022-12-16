#pragma once

#include "exceptions/assert_lite.hpp"
#include "lisp/assembly/amd64/inst/generated/jcc.hpp"
#include "lisp/assembly/amd64/inst/generated/setcc.hpp"

namespace j::lisp::assembly::amd64 {
  enum class condition_code : u8_t {
    /// Equal (Zero flag set)
    e,
    /// Equal (Zero flag set)
    z = e,

    /// Not equal (Zero flag clear)
    ne,
    /// Not equal (Zero flag clear)
    nz = ne,


    /// Unsigned: Above / not below or equal (carry and zero flags clear)
    a,
    /// Unsigned: Above / not below or equal (carry and zero flags clear)
    nbe = a,

    /// Unsigned: Below or equal / not above (carry or zero flag set)
    be,
    /// Unsigned: Below or equal / not above (carry or zero flag set)
    na = be,

    /// Unsigned: Above or equal / not below (carry flag clear)
    ae,
    /// Unsigned: Above or equal / not below (carry flag clear)
    nc = ae,
    /// Unsigned: Above or equal / not below (carry flag clear)
    nb = ae,

    /// Unsigned: Below / not above or equal (carry flag set)
    b,
    /// Unsigned: Below / not above or equal (carry flag set)
    c = b,
    /// Unsigned: Below / not above or equal (carry flag set)
    nae = b,

    /// Signed: Greater / not less than or equal (!ZF && SF == OF)
    g,
    /// Signed: Greater / not less than or equal (!ZF && SF == OF)
    nle = g,

    /// Signed: Greater than or equal / not less than (SF == OF)
    ge,
    /// Signed: Greater than or equal / not less than (SF == OF)
    nl = ge,

    /// Signed: Less than / not greater than or equal (SF != OF)
    l,
    /// Signed: Less than / not greater than or equal (SF != OF)
    nge = l,

    /// Signed: Less than or equal / not greater than (ZF && SF != OF)
    le,
    /// Signed: Less than or equal / not greater than (ZF && SF != OF)
    ng = le,



    /// Overflow flag set
    o,
    /// Overflow flag clear
    no,



    /// Sign flag set
    s,
    /// Sign flag clear
    ns,



    /// Parity flag set (even parity)
    p,
    /// Parity flag set (even parity)
    pe = p,

    /// Parity flag clear (odd parity)
    np,
    /// Parity flag clear (odd parity)
    po = np,
  };

  J_INLINE_GETTER instruction_encodings J_NOT_NULL jcc(condition_code c) noexcept {
    switch (c) {
    case condition_code::e:  return inst::je;
    case condition_code::ne: return inst::jne;
    case condition_code::a:  return inst::ja;
    case condition_code::be: return inst::jbe;
    case condition_code::ae: return inst::jae;
    case condition_code::b:  return inst::jb;
    case condition_code::g:  return inst::jg;
    case condition_code::ge: return inst::jge;
    case condition_code::l:  return inst::jl;
    case condition_code::le: return inst::jle;
    case condition_code::o:  return inst::jo;
    case condition_code::no: return inst::jno;
    case condition_code::s:  return inst::js;
    case condition_code::ns: return inst::jns;
    case condition_code::p:  return inst::jp;
    case condition_code::np: return inst::jnp;
    }
  }

  J_INLINE_GETTER instruction_encodings J_NOT_NULL setcc(condition_code c) noexcept {
    switch (c) {
    case condition_code::e:  return inst::setz;
    case condition_code::ne: return inst::setnz;
    case condition_code::a:  return inst::seta;
    case condition_code::be: return inst::setna;
    case condition_code::ae: return inst::setnc;
    case condition_code::b:  return inst::setc;
    case condition_code::g:  return inst::setnle;
    case condition_code::ge: return inst::setnl;
    case condition_code::l:  return inst::setl;
    case condition_code::le: return inst::setle;
    case condition_code::o:  return inst::seto;
    case condition_code::no: return inst::setno;
    case condition_code::s:  return inst::sets;
    case condition_code::ns: return inst::setns;
    case condition_code::p:  return inst::setp;
    case condition_code::np: return inst::setnp;
    }
  }
}
