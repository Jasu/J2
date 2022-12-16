#include "os/elf_reader.hpp"
#include "files/memory_mapping.hpp"
#include "logging/global.hpp"
#include "containers/trivial_array.hpp"
#include "containers/strided_span.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::os::elf64::section_info);

namespace j::os::elf64 {
  J_A(NODISC) const section_info * elf_info::find_named_section(named_section known_name) const noexcept {

    for (auto & sec : sections) {
      if (sec.known_name == known_name) {
        return &sec;
      }
    }
    return nullptr;
  }

  namespace s = strings;

  /// Info about a special section, this struct is not part of ELF.
  struct J_AT(HIDDEN) named_sec_info final {
    const char * name;
    const char * dwo_name;
    sec_type type;
    word flags;
  };

#define J_NAMED_WRAP(N) | sec_flag_##N
#define J_NAMED(N, T, ...) { #N, nullptr, sec_##T, 0U __VA_OPT__(J_FE(J_NAMED_WRAP, __VA_ARGS__)) },
#define J_NAMED_DWO(N, T, ...) { #N, #N".dwo", sec_##T, 0U __VA_OPT__(J_FE(J_NAMED_WRAP, __VA_ARGS__)) },
  static const named_sec_info named_sections[]{
    J_WITH_SPECIAL_ELF_SECTIONS(J_NAMED)
    J_WITH_DWARF_SECTIONS(J_NAMED, J_NAMED_DWO)
    { nullptr, nullptr, sec_null, 0 },
  };

  static named_section get_known_name(const char * J_AA(NN,NOALIAS) name) noexcept {
    ++name;
    const named_sec_info * J_AV(NOALIAS) it = named_sections;
    for (; it->name && strcmp(name, it->name) && (!it->dwo_name || strcmp(name, it->dwo_name)); ++it) { }
    return (named_section)(it - named_sections);
  }

  struct J_AT(HIDDEN) elf_data_access_impl final {
    files::memory_mapping mapping;
    elf_info * info = nullptr;

    template<typename T>
    J_A(AI,NODISC,ND,HIDDEN,RNN) const T * ptr_at(i32_t offset) const noexcept {
      return mapping.mapping.ptr_at<T>(offset);
    }
  };

  elf_reader::elf_reader(strings::const_string_view path)
    : impl(::new elf_data_access_impl{{path, files::map_full_range}})
  {
  }

  J_A(RNN,NODISC) const elf_info * elf_reader::read() noexcept {
    J_ASSERT_NOT_NULL(impl);
    J_ASSERT(!impl->info);
    const elf_header * ehdr = impl->ptr_at<elf_header>(0);
    J_DEBUG("EHDR Magic {} SHOFF: {} SHNUM: {} SHENTSZ: {} SHSTRNDX: {}",
            s::const_string_view((const char*)ehdr->ident, 4),
            ehdr->shoff, ehdr->shnum, ehdr->shentsize, ehdr->shstrndx);
    impl->info = ::new elf_info{
      { containers::uninitialized, ehdr->shnum },
      ehdr,
      impl->mapping.size(),
      ehdr->shstrndx,
    };
    strided_span<const elf_shdr> shdrs(impl->ptr_at<elf_shdr>(ehdr->shoff), ehdr->shnum, ehdr->shentsize);
    const elf_shdr & strtab = shdrs[ehdr->shstrndx];
    const char * strtab_begin = impl->ptr_at<char>(strtab.offset);
    for (const elf_shdr & shdr : shdrs) {
      // J_DEBUG("SHDR Name#: {} Offset: {}", shdr.name, shdr.offset);
      const char * name = strtab_begin + shdr.name;
      // J_DEBUG("     Name: {}", name);
      named_section known_name = get_known_name(name);
      // J_DEBUG("     Known: {}", (i32_t)known_name);
      impl->info->sections.brace_initialize_element(
        known_name,
        (i32_t)shdr.size,
        &shdr,
        impl->ptr_at<char>(shdr.offset),
        (i32_t)shdr.entsize,
        name);
    }
    return impl->info;
  }

  void elf_reader::close() noexcept {
    J_ASSERT_NOT_NULL(impl);
    ::delete impl->info;
    ::delete impl;
  }

  elf_reader::~elf_reader() {
    if (impl) {
      close();
    }
  }
}
