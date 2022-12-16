#include "os/dwarf/file_ctx.hpp"

namespace j::os::dwarf {
  J_A(NODISC) static sec_view to_sec_view(const elf64::elf_info * J_AA(NN) info, elf64::named_section name) {
    const elf64::section_info * sec = info->find_named_section(name);
    return sec ? sec_view{ sec->section, sec->section + sec->size } : sec_view{};
  }

  file_ctx::file_ctx(strings::const_string_view path)
    : elf(path)
  {
    auto info = elf.read();
    debug_info = to_sec_view(info, elf64::named_sec_debug_info);
    debug_abbrev = to_sec_view(info, elf64::named_sec_debug_abbrev);
    debug_str = to_sec_view(info, elf64::named_sec_debug_str);
    debug_str_offsets = to_sec_view(info, elf64::named_sec_debug_str_offsets);
    debug_line = to_sec_view(info, elf64::named_sec_debug_line);
    debug_line_str = to_sec_view(info, elf64::named_sec_debug_line_str);
    debug_rnglists = to_sec_view(info, elf64::named_sec_debug_rnglists);
    debug_addr = to_sec_view(info, elf64::named_sec_debug_addr);
    debug_loclists = to_sec_view(info, elf64::named_sec_debug_loclists);

    J_REQUIRE(debug_info && debug_abbrev, ".debug_info or .debug_abbrev is missing from {}.", path);
  }
}
