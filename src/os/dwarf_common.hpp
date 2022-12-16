#pragma once

#include "hzd/utility.hpp"

#define J_DWARF_UNDEF(...)

namespace j::os::dwarf {
  enum format : u8_t {
    dwarf32,
    dwarf64,
  };

  using sbyte = i8_t;
  using ubyte = u8_t;

#define J_DWARF_UNION(NAME,NUM) \
  union J_AT(PACK) NAME { \
    NUM value; \
    J_A(AI,ND,NODISC,HIDDEN) inline operator NUM() const noexcept { \
      return value; \
    } \
  }; \
  static_assert(alignof(NAME) == 1)

  J_DWARF_UNION(uhalf, u16_t);
  J_DWARF_UNION(shalf, i16_t);
  J_DWARF_UNION(uword, u32_t);
  J_DWARF_UNION(sword, i32_t);
  J_DWARF_UNION(udword, u64_t);
  J_DWARF_UNION(sdword, i64_t);

  template<format> struct J_AT(PACK) off;

  using off_32 = off<dwarf32>;
  using off_64 = off<dwarf64>;

  template<format> struct J_AT(PACK,PREF_NAME(off_32),PREF_NAME(off_64)) off;

  template<> struct J_AT(PACK) off<dwarf32> final {
    u32_t value;
    J_A(AI,ND,NODISC) inline operator u64_t() const noexcept {
      return value;
    }
  };

  template<> struct J_AT(PACK) off<dwarf64> final {
    u64_t value;
    J_A(AI,ND,NODISC) inline operator u64_t() const noexcept {
      return value;
    }
  };

  enum dwarf_section : u8_t {
    sec_debug_info,
    sec_debug_str,
    sec_debug_str_offsets,
    sec_debug_line,
    sec_debug_line_str,
    sec_debug_addr,
    sec_debug_aranges,
    sec_debug_abbrev,
    sec_debug_loclists,
    sec_debug_rnglists,
    sec_debug_none,
  };

  J_A(HIDDEN) inline constexpr i32_t num_dwarf_sections = (i32_t)sec_debug_none;

  template<format> struct initial_length;

  template<> struct J_AT(PACK) initial_length<dwarf32>;
  template<> struct J_AT(PACK) initial_length<dwarf64>;

  using initial_length_32 = initial_length<dwarf32>;
  using initial_length_64 = initial_length<dwarf64>;

  template<format> struct J_AT(PREF_NAME(initial_length_64),PREF_NAME(initial_length_32)) initial_length;

  template<>
  struct J_AT(PACK) initial_length<dwarf32> final {
    uword len;

    J_A(AI,ND,NODISC) inline operator u64_t() const noexcept {
      return len.value;
    }
  };
  template<>
  struct J_AT(PACK) initial_length<dwarf64> final {
    uword init;
    udword len;

    J_A(AI,ND,NODISC) inline operator u64_t() const noexcept {
      return len.value;
    }
  };

}
