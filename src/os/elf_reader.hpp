#pragma once

#include "os/elf.hpp"
#include "os/elf_section.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::os::elf64 {
  struct J_AT(HIDDEN) section_info final {
    named_section known_name;
    i32_t size;
    const elf_shdr * shdr;
    const char * section;
    i32_t entsize;
    const char * name;
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::os::elf64::section_info);

namespace j::os::elf64 {
  struct J_AT(HIDDEN) elf_info final {
    containers::trivial_array<section_info> sections;
    const elf_header * elf_begin;
    i32_t size;
    i32_t section_strtab_index;
    J_A(NODISC) const section_info * find_named_section(named_section known_name) const noexcept;
  };

  struct J_AT(HIDDEN) elf_data_access_impl;

  struct elf_reader {
    J_A(AI,ND,HIDDEN) inline elf_reader() noexcept = default;

    elf_reader(strings::const_string_view path);

    J_A(RNN,NODISC) const elf_info *read() noexcept;

    void close() noexcept;

    ~elf_reader();

    J_A(AI,HIDDEN,ND) inline elf_reader(elf_reader && rhs) noexcept
      : impl(rhs.impl)
    {
      rhs.impl = nullptr;
    }

    J_A(AI,HIDDEN,ND) inline elf_reader & operator=(elf_reader && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (J_UNLIKELY(impl)) {
          close();
        }
        impl = rhs.impl;
        rhs.impl = nullptr;
      }
      return *this;
    }


  private:
    elf_data_access_impl* impl = nullptr;
  };
}
