#pragma once

#include "os/elf_common.hpp"

namespace j::os::elf64 {
  /// Section header
  struct elf_shdr final {
    /// Index to the string table specified by ehdr.shstrndx
    word name;
    /// Section type
    sec_type type;
    /// Section flags
    xword flags;
    /// If the section is allocated when runtime-linked, this is its address or offset
    addr address;
    /// Byte offset from beginning of the whole file to the section.
    off offset;
    xword size;
    /// Dynamic: refers to string table used for string values in the dynamic table.
    /// Hash: section index of the symbol table this is for
    /// REL and RELA: section index of the symbol table used for the relocations
    /// Symtab and Dynsym: section index of the string table used for symbol names.
    /// Group: section index of the symbol table uesd for the group
    /// Symtab SHNDX section index of the associated symbol table
    word link;
    /// REL and RELA: Section index of the section to apply the relocs to
    /// Symtab and Dynsym: Largest symbol table index referenced by local symbols in this section
    /// Group: Index of a symbol in the table specified by `link`, meaning the signature of the group.
    word info;
    /// If set to a power of two not 0 or 1, the section must be aigned
    /// to that alignment when mapped.
    xword addralign;
    /// Size of fixed-size entities in the table. For some string tables, char width for wchars.
    /// Otherwise zero.
    xword entsize;
  };

#define J_WITH_SPECIAL_ELF_SECTIONS(M)    \
  M(bss, nobits, alloc, write)                  \
  M(comment, progbits)                          \
  M(data, progbits, alloc, write)               \
  M(debug, progbits)                            \
  M(dynamic, dynamic)                           \
  M(dynstr, strtab, alloc)                      \
  M(dynsym, dynsym, alloc)                      \
  M(fini, progbits, alloc, execinstr)           \
  M(fini_array, fini_array, alloc, write)       \
  M(init, progbits, alloc, execinstr)           \
  M(init_array, init_array, alloc, write)       \
  M(preinit, progbits, alloc, execinstr)        \
  M(preinit_array, preinit_array, alloc, write) \
  M(interp, progbits)                           \
  M(line, progbits)                             \
  M(note, note)                                 \
  M(plt, progbits)                              \
  M(got, progbits)                              \
  M(hash, hash, alloc)                          \
  M(rodata, progbits, alloc)                    \
  M(shstrtab, strtab)                           \
  M(strtab, strtab)                             \
  M(symtab, symtab)                             \
  M(symtab_shndx, symtab_shndx)                 \
  M(tbss, nobits, alloc, write, tls)            \
  M(tdata, progbits, alloc, write, tls)         \
  M(text, progbits, alloc, execinstr)

#define J_WITH_DWARF_SECTIONS(M,M_DWO)          \
  M_DWO(debug_info, progbits)                       \
  M_DWO(debug_str, progbits, strings, merge)        \
  M_DWO(debug_str_offsets, progbits)                \
  M_DWO(debug_line, progbits)                       \
  M_DWO(debug_loclists, progbits)                   \
  M(debug_line_str, progbits, strings, merge)   \
  M_DWO(debug_rnglists, progbits)                   \
  M(debug_aranges, progbits)                    \
  M(debug_addr, progbits)                       \
  M_DWO(debug_abbrev, progbits)                     \
  M_DWO(debug_macro, progbits)                      \
  M(debug_names, progbits)                      \
  M(debug_frame, progbits)

#define J_ENUM_VAL(N, ...) named_sec_##N,
  enum named_section {
    J_WITH_SPECIAL_ELF_SECTIONS(J_ENUM_VAL)
    J_WITH_DWARF_SECTIONS(J_ENUM_VAL, J_ENUM_VAL)
    named_sec_none,
    named_sec_size = named_sec_none,
  };
#undef J_ENUM_VAL
}
