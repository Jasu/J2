#pragma once

#include "os/dwarf_reader_base.hpp"
#include "os/elf_reader.hpp"

namespace j::os::dwarf {
  struct sec_view final {
    const char * begin = nullptr;
    const char * end = nullptr;

    J_A(AI,ND,NODISC) inline bool operator!() const noexcept { return begin == end; }
    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return begin != end; }

    J_A(AI,NODISC,HIDDEN,ND) inline base_reader get_reader(u32_t offset) {
      J_ASSERT(begin + offset < end);
      return {begin + offset, end};
    }

    J_A(AI,NODISC,HIDDEN,ND) inline base_reader get_reader(const char * J_AA(NN) offset) {
      J_ASSERT(begin <= offset && offset < end);
      return {offset, end};
    }

    J_A(AI,NODISC,HIDDEN,ND) inline base_reader get_reader() {
      J_ASSERT(begin != end);
      return {begin, end};
    }
  };

  struct file_ctx final {
    inline file_ctx() noexcept = default;
    explicit file_ctx(strings::const_string_view path);

    sec_view debug_info;
    sec_view debug_abbrev;
    sec_view debug_str;
    sec_view debug_str_offsets;
    sec_view debug_line;
    sec_view debug_line_str;
    sec_view debug_rnglists;
    sec_view debug_addr;
    sec_view debug_loclists;

    elf64::elf_reader elf;
  };
}
