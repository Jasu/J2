#pragma once

#include "os/dwarf_common.hpp"

#define J_WITH_DWARF_FORMS(M) \
  M(addr,       0x01) \
  M(block2,     0x03) \
  M(block4,     0x04) \
  M(data2,      0x05) \
  M(data4,      0x06) \
  M(data8,      0x07) \
  M(string,     0x08) \
  M(block,      0x09) \
  M(block1,     0x0a) \
  M(data1,      0x0b) \
  M(flag,       0x0c) \
  M(sdata,      0x0d) \
  M(strp,       0x0e) \
  M(udata,      0x0f) \
  M(ref_addr,   0x10) \
  M(ref1,       0x11) \
  M(ref2,       0x12) \
  M(ref4,       0x13) \
  M(ref8,       0x14) \
  M(ref_udata,  0x15) \
  M(indirect,   0x16) \
  M(sec_off,    0x17) \
  M(exprloc,    0x18) \
  M(flag_set,   0x19) \
  M(strx,       0x1a) \
  M(addrx,      0x1b) \
  M(ref_sup4,   0x1c) \
  M(strp_sup,   0x1d) \
  M(data16,     0x1e) \
  M(line_strp,  0x1f) \
  M(ref_sig8,   0x20) \
  M(impl_const, 0x21) \
  M(loclistx,   0x22) \
  M(rnglistx,   0x23) \
  M(ref_sup8,   0x24) \
  M(strx1,      0x25) \
  M(strx2,      0x26) \
  M(strx3,      0x27) \
  M(strx4,      0x28) \
  M(addrx1,     0x29) \
  M(addrx2,     0x30) \
  M(addrx3,     0x31) \
  M(addrx4,     0x32)

namespace j::os::dwarf {
  enum dw_form : u8_t {
#define J_FORM_ENUM(N, V) dw_form_##N = V,
    J_WITH_DWARF_FORMS(J_FORM_ENUM)
  };
}
