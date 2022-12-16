#pragma once

#include "hzd/utility.hpp"
#include "os/dwarf_common.hpp"

#define J_WITH_DWARF_ATTRIBUTES(M, ...) J_WITH_DWARF_ATTRIBUTES_(M, __VA_OPT__(__VA_ARGS__ ,) J_DWARF_UNDEF)
#define J_WITH_DWARF_ATTRIBUTES_(M, U, ...)          \
  M(sibling,     0x01, ref) \
  M(location,    0x02, exprloc, loclist) \
  M(name,        0x03, str) \
  M(ordering,    0x09, const) \
  M(byte_size,   0x0b, const,   exprloc, ref) \
  M(bit_size,    0x0d, const,   exprloc, ref) \
  M(stmt_list,   0x10, lineptr) \
  M(pc_low,      0x11, addr) \
  M(pc_high,     0x12, addr,    const) \
  M(language,    0x13, const) \
  U(,discr,       0x15, ref, "Unsupported: Pascal (and other) discriminated unions") \
  U(,discr_value, 0x16, const, "Unsupported: Pascal (and other) discriminated unions") \
  M(visibility,  0x17, const) \
  M(import,      0x18, ref) \
  M(strlen,      0x19, exprloc, loclist, const) \
  U(, common_reference,  0x1a, "Unsupported: Fortran")                 \
  M(comp_dir,    0x1b, str) \
  M(const_val,   0x1c, block,   const,   str) \
  M(cont_type,   0x1d, ref) \
  M(def_val,     0x1e, const,   ref,     flag) \
  M(inline,      0x20, const) \
  M(is_opt,      0x21, flag) \
  M(bnd_lo,      0x22, const,   exprloc, ref) \
  M(producer,    0x25, str) \
  M(prototyped,  0x27, flag) \
  M(ret_addr,    0x2a, exprloc, loclist) \
  M(s_scope,     0x2c, const,   rnglist) \
  M(bit_stride,  0x2e, const,   exprloc, ref) \
  M(bnd_hi,      0x2f, const,   exprloc, ref) \
  M(abstract_origin,  0x31, ref) \
  M(access,      0x32, const) \
  M(addr_cls,    0x33, const) \
  M(artificial,  0x34, flag) \
  M(base_types,  0x35, ref) \
  M(callconv,    0x36, const) \
  M(count,       0x37, const,   exprloc, ref) \
  M(datamem_loc, 0x38, const,   exprloc, loclist) \
  M(decl_col,    0x39, const) \
  M(decl_file,   0x3a, const) \
  M(decl_line,   0x3b, const) \
  M(declaration,        0x3c, flag) \
  U(,discr_list,  0x3d, block, "Unsupported: Pascal (and other) discriminated unions") \
  M(encoding,    0x3e, const) \
  M(external,    0x3f, flag) \
  M(frame_b,     0x40, exprloc,loclist) \
  M(friend,      0x41, ref) \
  U(,identifier_case,     0x42, const,   "Unused: Identifier case sensitivity") \
  U(,nlistit,     0x44, ref,     "Unused: Fortran")                     \
  U(,priority,    0x45, ref, "Unused: Module support")                  \
  U(,segment,     0x46, exprloc,loclist, "Unused: Segmented addressing") \
  M(specification,        0x47, ref) \
  M(staticlink,  0x48, exprloc,loclist) \
  M(type,        0x49, ref) \
  M(use_loc,     0x4a, exprloc,loclist) \
  U(, variable_parameter,   0x4b, flag, "Unused: Indicates parameter modifiable in calling scope")        \
  M(virtuality,  0x4c, const) \
  M(vtbl_el_loc, 0x4d, exprloc,loclist) \
  U(,allocated,   0x4e, const,   exprloc, ref, "Unused: Fortran") \
  U(,associated,  0x4f, const,   exprloc, ref, "Unused: Fortran") \
  M(data_loc,    0x50, exprloc) \
  M(byte_stride, 0x51, const,   exprloc, ref) \
  M(entry_pc,    0x52, addr,    const) \
  U(, use_utf8,    0x53, flag,    "Unused: UTF8 assumed")  \
  M(extension,   0x54, ref) \
  M(ranges,      0x55, rnglist)  \
  M(trampoline,  0x56, addr,    flag,    ref,str)           \
  M(c_col,       0x57, const) \
  M(c_file,      0x58, const) \
  M(c_line,      0x59, const) \
  M(desc,        0x5a, str) \
  U(, bin_scl,     0x5b, const,   "Unused: scaled encoding") \
  U(, dec_scl,     0x5c, const,   "Unused: Dec string")     \
  U(, small,       0x5d, ref,     "Unused: ADA")               \
  U(, dec_sign,    0x5e, const,   "Unused: COBOL")                \
  U(, digit_cnt,   0x5f, const,   "Unused: COBOL")                       \
  U(, pic_string,  0x60, str,     "Unused: COBOL")                       \
  M(mutable,     0x61, flag)                                           \
  U(, thread_scl,  0x62, flag,    "Unused: Likely unused")               \
  M(explicit,    0x63, flag)         \
  M(obj_ptr,     0x64, ref)         \
  M(endian,      0x65, const) \
  U(, elemental,   0x66, flag,    "Unused: FORTRAN")                   \
  U(, pure,        0x67, flag,    "Unused: FORTRAN")                    \
  U(, recursive,   0x68, flag,    "Unused: FORTRAN")                     \
  M(signature,   0x69, ref)         \
  M(main_func,   0x6a, flag)         \
  M(data_bitoff, 0x6b, const) \
  M(const_expr,  0x6c, flag)         \
  M(enum_cls,    0x6d, flag)         \
  M(link_name,   0x6e, str)         \
  M(strl_bits,   0x6f, const) \
  M(strl_bytes,  0x70, const) \
  M(rank,        0x71, const,   exprloc)                   \
  M(stroff_b,    0x72, stroffsetsptr)                   \
  M(addr_b,      0x73, addrptr)                   \
  M(rnglists_b,  0x74, rnglistsptr)                   \
  M(dwo_name,    0x76, str)                   \
  M(reference,   0x77, flag)                   \
  M(rvalue_ref,  0x78, flag)                   \
  M(macros,      0x79, macptr)                   \
  M(c_all,       0x7a, flag)                   \
  M(c_all_src,   0x7b, flag)                   \
  M(c_all_tail,  0x7c, flag)                   \
  M(c_ret_pc,    0x7d, addr)                   \
  M(c_value,     0x7e, exprloc)                   \
  M(c_origin,    0x7f, exprloc)                   \
  M(c_param,     0x80, ref)                   \
  M(c_pc,        0x81, addr)                   \
  M(c_tail,      0x82, flag)                   \
  M(c_target,    0x83, exprloc)                   \
  M(c_clobber,   0x84, exprloc)                   \
  M(c_dataloc,   0x85, exprloc)                   \
  M(c_dataval,   0x86, exprloc)                   \
  M(noreturn,    0x87, flag)                   \
  M(alignment,   0x88, const)                   \
  M(export_syms, 0x89, flag)                   \
  M(deleted,     0x8a, flag)                   \
  M(defaulted,   0x8b, const)                   \
  M(loclists_b,  0x8c, loclistsptr)                   \

namespace j::os::dwarf {
  enum dw_at : u8_t {
#define J_ATTR_ENUM(N, V, ...) dw_at_##N = V,
    J_WITH_DWARF_ATTRIBUTES(J_ATTR_ENUM)
  };

  enum dw_inl : u8_t {
    dw_inl_not_inlined,
    dw_inl_inlined,
    dw_inl_declared_not_inlined,
    dw_inl_declared_inlined,
  };
}
