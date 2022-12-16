#pragma once

#include "os/dwarf_common.hpp"

#define J_WITH_DWARF_TAGS(M, ...) J_WITH_DWARF_TAGS_(M, __VA_OPT__(__VA_ARGS__ ,) J_DWARF_UNDEF)
#define J_WITH_DWARF_TAGS_(M, U, ...)   \
  M(array_type,               0x01) \
  M(class_type,               0x02) \
  M(entry_point,              0x03) \
  M(enumeration_type,         0x04) \
  M(formal_parameter,         0x05) \
  M(imported_declaration,     0x08) \
  M(label,                    0x0a) \
  M(lexical_block,            0x0b) \
  M(member,                   0x0d) \
  M(pointer_type,             0x0f) \
  M(reference_type,           0x10) \
  M(compile_unit,             0x11) \
  U(string_type,              0x12, "Unsupported: Fortran")            \
  M(structure_type,           0x13) \
  M(subroutine_type,          0x15) \
  M(typedef,                  0x16) \
  M(union_type,               0x17) \
  M(unspecified_parameters,   0x18) \
  U(variant,                  0x19, "Unsupported: Pascal (and other) discriminated unions") \
  U(common_block,             0x1a, "Unsupported: Fortran")            \
  U(common_inclusion,         0x1b, "Unsupported: Fortran") \
  M(inheritance,              0x1c) \
  M(inlined_subroutine,       0x1d) \
  M(module,                   0x1e) \
  M(ptr_to_member_type,       0x1f) \
  U(set_type,                 0x20, "Unsupported: Pascal")            \
  M(subrange_type,            0x21) \
  U(with_stmt,                0x22, "Unsupported: Modula-2")  \
  M(access_declaration,       0x23) \
  M(base_type,                0x24) \
  M(catch_block,              0x25) \
  M(const_type,               0x26) \
  U(constant,                 0x27, "Unsupported: Named constant, i.e. something that does not have a concrete location, not in C") \
  M(enumerator,               0x28) \
  U(file_type,                0x29, "Unsupported: Pascal")  \
  M(friend,                   0x2a) \
  U(namelist,                 0x2b, "Unsupported: Fortran")  \
  U(namelist_item,            0x2c, "Unsupported: Fortran")           \
  U(packed_type,              0x2d, "Unsupported: Ada/Pascal")                                  \
  M(subprogram,               0x2e) \
  M(template_type_parameter,  0x2f) \
  M(template_value_parameter, 0x30) \
  M(thrown_type,              0x31) \
  M(try_block,                0x32) \
  U(variant_part,             0x33, "Unsupported: Pascal (and other) discriminated unions") \
  M(variable,                 0x34) \
  M(volatile_type,            0x35) \
  M(dwarf_procedure,          0x36) \
  M(restrict_type,            0x37) \
  U(interface_type,           0x38, "Unsupported: Java")  \
  M(namespace,                0x39) \
  M(imported_module,          0x3a) \
  M(unspecified_type,         0x3b) \
  M(partial_unit,             0x3c) \
  M(imported_unit,            0x3d) \
  U(condition,                0x3f, "Unsupported: COBOL")                     \
  U(shared_type,              0x40, "Unsupported: UPC")  \
  M(type_unit,                0x41) \
  M(rvalue_reference_type,    0x42) \
  M(template_alias,           0x43) \
  U(coarray_type,             0x44, "Unsupported: Fortran")            \
  M(generic_subrange,         0x45) \
  U(dynamic_type,             0x46, "Unsupported: Dynamically typed languages")            \
  M(atomic_type,              0x47) \
  M(call_site,                0x48) \
  M(call_site_parameter,      0x49) \
  M(skeleton_unit,            0x4a) \
  U(immutable_type,           0x4b, "Unsupported: D")  \

namespace j::os::dwarf {
  enum dw_tag : u8_t {
#define J_TAG_ENUM(N, V, ...) dw_tag_##N = V,
    J_WITH_DWARF_TAGS(J_TAG_ENUM)
  };
}
