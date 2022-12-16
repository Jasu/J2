#pragma once

#include <detail/preamble.hpp>

#include "lisp/assembly/buffer.hpp"
#include "lisp/assembly/amd64/inst.hpp"
#include "strings/format.hpp"

namespace a = j::lisp::assembly;
namespace x = j::lisp::assembly::amd64;
namespace i = j::lisp::assembly::amd64::inst;

constexpr inline const char padding[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

inline const j::strings::const_string_view get_test_fixture(const u8_t * fixture, u32_t i) noexcept {
  return { (const char*)fixture + i * 16U, 16U };
}

template<typename... Operands>
inline j::strings::string debug_message(
  j::strings::const_string_view expected,
  const j::strings::string & emitted,
  Operands && ... ops)
{
  return j::strings::format("Emitted {hex}, expected {hex}. Operands:",
                            emitted, expected,
                            static_cast<Operands &&>(ops)...);
}

template<typename Inst, typename... Ts>
inline void test_emit(
  const j::strings::const_string_view expected,
  const Inst & inst, Ts && ... ts)
{
  j::lisp::assembly::buffer buf;
  buf.emit(inst, static_cast<Ts &&>(ts)...);
  if (const u32_t num_padding = (16U - (buf.size() & 0xFU)) & 0xFU) {
    buf.append(padding, num_padding);
  }
  const auto emitted = buf.build();
  CAPTURE(debug_message(expected, emitted, static_cast<Ts &&>(ts)...));
  REQUIRE(expected == emitted);
}

template<typename Fn, typename... Args>
void with_reg(a::mem_width sz, Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(a::gpr(i, sz), static_cast<Args &&>(args)...);
  }
}

template<typename... Args>
J_ALWAYS_INLINE void with_reg(u8_t sz, Args && ... args)
{ return with_reg(a::width(sz), static_cast<Args &&>(args)...); }

template<typename Fn, typename... Args>
void with_reg_reg(u8_t sz_lhs, u8_t sz_rhs, Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    for (u32_t j = 0; j < 16; ++j) {
      static_cast<Fn &&>(fn)(a::gpr(j, a::width(sz_lhs)),
                             a::gpr(i, a::width(sz_rhs)),
                             static_cast<Args &&>(args)...);
    }
  }
}

template<typename Fn, typename... Args>
void with_reg8(Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(a::gpr8(i), static_cast<Args &&>(args)...);
  }
}

template<typename Fn, typename... Args>
void with_reg16(Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(a::gpr16(i), static_cast<Args &&>(args)...);
  }
}

template<typename Fn, typename... Args>
void with_reg32(Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(a::gpr32(i), static_cast<Args &&>(args)...);
  }
}

template<typename Fn, typename... Args>
void with_reg64(Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(a::gpr64(i), static_cast<Args &&>(args)...);
  }
}

template<typename Fn, typename... Args>
void with_xmms(Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(x::xmm(i), static_cast<Args &&>(args)...);
  }
}

template<typename Fn, typename... Args>
void with_ymms(Fn && fn, Args && ... args) {
  for (u32_t i = 0; i < 16; ++i) {
    static_cast<Fn &&>(fn)(x::ymm(i), static_cast<Args &&>(args)...);
  }
}

template<typename Fn>
J_ALWAYS_INLINE void with_all_gprs(Fn && fn) {
  for (a::mem_width w : a::widths) {
    with_reg(w, static_cast<Fn &&>(fn));
  }
}

template<typename Fn>
J_ALWAYS_INLINE void with_all_gprs_size(Fn && fn) {
  for (a::mem_width w : a::widths) {
    with_reg(w, static_cast<Fn &&>(fn), w);
  }
}

template<typename Fn>
void with_all_gprs_immsize(Fn && fn) {
  with_reg8(static_cast<Fn &&>(fn),  a::mem_width::byte);
  with_reg16(static_cast<Fn &&>(fn), a::mem_width::word);
  with_reg32(static_cast<Fn &&>(fn), a::mem_width::dword);
  with_reg64(static_cast<Fn &&>(fn), a::mem_width::dword);
}
template<typename Fn>
void with_all_gprs_size_immsize(Fn && fn) {
  with_reg8(static_cast<Fn &&>(fn),  a::mem_width::byte,  a::mem_width::byte);
  with_reg16(static_cast<Fn &&>(fn), a::mem_width::word,  a::mem_width::word);
  with_reg32(static_cast<Fn &&>(fn), a::mem_width::dword, a::mem_width::dword);
  with_reg64(static_cast<Fn &&>(fn), a::mem_width::qword, a::mem_width::dword);
}

template<typename Fn>
J_ALWAYS_INLINE void with_size(Fn && fn) {
  for (a::mem_width w : a::widths) {
    static_cast<Fn &&>(fn)(w);
  }
}
template<typename Fn>
J_ALWAYS_INLINE void with_size_immsize(Fn && fn) {
  static_cast<Fn &&>(fn)(a::mem_width::byte,  a::mem_width::byte);
  static_cast<Fn &&>(fn)(a::mem_width::word,  a::mem_width::word);
  static_cast<Fn &&>(fn)(a::mem_width::dword, a::mem_width::dword);
  static_cast<Fn &&>(fn)(a::mem_width::qword, a::mem_width::dword);
}
