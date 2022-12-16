#pragma once

#include "os/elf_common.hpp"

namespace j::os::elf64 {
  constexpr inline u8_t ident_val[ei_nident] = {
    0x7f, 'E', 'L', 'F',
    elf_64,
    elf_little_endian,
    elf_ver_current,
    elf_osabi_plain,
    0, // ABI version
    0, 0, 0, 0, 0, 0, 0
  };

  struct elf_header final {
    /// Magic string and ABI details
    u8_t ident[ei_nident];
    /// Whether a shared library, object file, executable, coredump...
    elf_type type;
    elf_machine machine;
    word version;
    /// Entry point address for executables (offset for relocatable executables)
    addr entry;
    /// Program header offset in bytes
    off phoff;
    /// Section header offset in bytes
    off shoff;
    /// Processor flags
    word flags;
    /// Size of this ELF header.
    half ehsize;
    /// Size of a PHDR entry
    half phentsize;
    /// Number of PHDR entries
    half phnum;
    /// Section header byte size
    half shentsize;
    /// Number of sections
    half shnum;
    /// Index of the section containing the string table used by SHDR root
    half shstrndx;
  };

  /// Segment header (program header)
  struct elf_phdr final {
    /// Segment type.
    seg_type type;
    /// Bitflags
    word flags;
    /// Offset from the start of the file where the segment resides.
    off offset;
    /// Virtual address of the segment.
    addr vaddr;
    /// Physical address of the segment. Unused.
    addr paddr;
    /// Size of the segment on disk.
    xword filesz;
    /// Size of the segment when loaded.
    xword memsz;
    xword size;
    /// Alignment when loaded (due to memory mapping must divide `offset - vaddr`.
    xword align;
  };
}
