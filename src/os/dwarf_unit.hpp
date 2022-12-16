#pragma once

#include "os/dwarf_common.hpp"

namespace j::os::dwarf {
  enum unit_type : ubyte {
    ut_none,
    ut_compile = 1,
    ut_type = 2,
    ut_partial = 3,
    ut_skeleton = 4,
    ut_split_compile = 5,
    ut_split_type = 6,
  };

  template<format F> struct J_AT(PACK) dw_ut;

  using dw_ut_32 = dw_ut<dwarf32>;
  using dw_ut_64 = dw_ut<dwarf64>;

  template<format F> struct J_AT(PACK,PREF_NAME(dw_ut_32),PREF_NAME(dw_ut_64)) dw_ut {
    initial_length<F> unit_length;
    uhalf ver;
    ubyte unit_type;
    ubyte addr_size;
    off<F> abbrev_offset;
  };

  template<format F> struct J_AT(PACK) dw_ut_skeleton;

  using dw_ut_skeleton_32 = dw_ut_skeleton<dwarf32>;
  using dw_ut_skeleton_64 = dw_ut_skeleton<dwarf64>;

  template<format F> struct J_AT(PACK,PREF_NAME(dw_ut_skeleton_32),PREF_NAME(dw_ut_skeleton_64)) dw_ut_skeleton : dw_ut<F> {
    udword dwo_id;
  };

  static_assert(sizeof(dw_ut_skeleton_32) == sizeof(dw_ut_32) + 8);
  static_assert(sizeof(dw_ut_skeleton_64) == sizeof(dw_ut_64) + 8);

  template<format F> struct J_AT(PACK) dw_ut_type;

  using dw_ut_type_32 = dw_ut_type<dwarf32>;
  using dw_ut_type_64 = dw_ut_type<dwarf64>;

  template<format F> struct J_AT(PACK,PREF_NAME(dw_ut_type_32),PREF_NAME(dw_ut_type_64)) dw_ut_type : dw_ut<F> {
    udword type_signature;
    off<F> type_offset;
  };

  struct dwarf_unit_header_info final {
    u64_t unit_length;
    u32_t data_length;
    u8_t ver;
    u8_t unit_type;
    u8_t addr_size;
    format format;
    union {
      u64_t type_signature;
      u64_t dwo_id;
    };
    u64_t type_offset;
  };
}
