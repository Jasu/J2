#include "os/dl.hpp"
#include "exceptions/assert.hpp"

#include <link.h>


namespace s = j::strings;

namespace j::os {
  namespace {
    [[nodiscard]] Elf64_Addr find_dyn(const Elf64_Dyn * J_NOT_NULL J_RESTRICT dyn, u32_t tag) noexcept {
      for (; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == tag) {
          return dyn->d_un.d_ptr;
        }
      }
      return 0;
    }

    int iter_cb(struct dl_phdr_info *info, [[maybe_unused]] sz_t size, void * J_RESTRICT data) {
      loaded_library * l = reinterpret_cast<loaded_library*>(data);
      J_ASSERT_NOT_NULL(info, l);
      Dl_info dl_info{};
      link_map * lm = nullptr;
      if (!dladdr1((const void*)info->dlpi_addr, &dl_info, (void**)&lm, RTLD_DL_LINKMAP)) {
        J_THROW("dladdr1({}) failed", (void*)info->dlpi_addr);
      }
      if (!lm || !lm->l_ld) {
        return 0;
      }

      auto strtab_offset = find_dyn(lm->l_ld, DT_STRTAB);
      if (strtab_offset < 0x4000) {
        return 0;
      }
      uint32_t soname_offset = (uint32_t)find_dyn(lm->l_ld, DT_SONAME);
      if (reinterpret_cast<const char*>(strtab_offset + soname_offset) != l->soname) {
        return 0;
      }
      l->path = info->dlpi_name;
      l->loaded_at = (const void*)info->dlpi_addr;
      return 1;
    }
  }

  [[nodiscard]] loaded_library get_loaded_library(strings::const_string_view soname) {
    J_ASSERT(soname);
    loaded_library result{soname};
    dl_iterate_phdr(&iter_cb, &result);
    if (!result) {
      result.soname.clear();
    }
    return result;
  }
}
