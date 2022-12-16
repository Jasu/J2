#include <link.h>

#include "logging/global.hpp"
#include "cli/cli.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::misc {
  namespace {
    struct J_TYPE_HIDDEN ctx {
      mem::shared_ptr<strings::formatted_sink> out;
    };

    int iter_cb(struct dl_phdr_info *info, [[maybe_unused]] sz_t size, void *data) {
      ctx * c = reinterpret_cast<ctx*>(data);
      J_ASSERT_NOT_NULL(info, c);
      c->out->write("Object {#bold}{}{/} at {#bright_cyan}0x{:012X}{/}\n",
                    info->dlpi_name, (uptr_t)info->dlpi_addr);
      for (sz_t i = 0; i < info->dlpi_phnum; ++i) {
        auto & phdr = info->dlpi_phdr[i];
        switch (phdr.p_type) {
        case PT_LOAD:         c->out->write("  {#bright_green,bold}Loadable {/}"); break;
        case PT_DYNAMIC: c->out->write("  {#bright_yellow,bold}DynInfo  {/}"); break;
        case PT_INTERP:       c->out->write("  {#bright_cyan,bold}Interp   {/}"); break;
        case PT_NOTE:         c->out->write("  {#bright_magenta,bold}Note     {/}"); break;
        case PT_TLS:          c->out->write("  {#bright_blue,bold}TLS      {/}"); break;
        case PT_PHDR:         c->out->write("  {#bold}PHDR     {/}"); break;
        case PT_GNU_RELRO:    c->out->write("  {#bright_green,bold}.relro   {/}"); break;
        case PT_GNU_EH_FRAME: c->out->write("  {#bright_cyan,bold}.eh_frame{/}"); break;
        case PT_GNU_STACK:    c->out->write("  {#bright_red,bold}ExecStack{/}"); break;
        case PT_SHLIB:        c->out->write("  {#bright_red,bold}SHLIB    {/}"); break;
        default:              c->out->write("  {#bright_red,bold}Unknown  {/}"); break;
        }
        c->out->write(" at {#bright_cyan,bold}0x{:012X}{/}, file offset {#bright_yellow}0x{:06X}{/}\n", (uptr_t)phdr.p_vaddr, (uptr_t)phdr.p_offset);
      }
      c->out->write("\n");

      link_map * lm = nullptr;
      Dl_info dl_info{};
      if (!dladdr1((const void*)info->dlpi_addr, &dl_info, (void**)&lm, RTLD_DL_LINKMAP)) {
        c->out->write("  {#error} dladdr1 failed. {/}\n");
        return 0;
      }
      if (!lm || !lm->l_ld) {
        c->out->write("  {#error} No dynamic map from dladdr1. {/}\n");
        return 0;
      }

      c->out->write("  {#tag1} Dynamic symbols: {/}\n");
      const char * strtab_addr = (const char*)info->dlpi_addr;
      for (auto d = lm->l_ld; d->d_tag != DT_NULL; ++d) {
        if (d->d_tag == DT_STRTAB) {
          if (d->d_un.d_ptr < 0x1000) {
            break;
          }
          strtab_addr = (const char*)d->d_un.d_ptr;
          break;
        }
      }

      for (auto d = lm->l_ld; d->d_tag != DT_NULL; ++d) {
        void * addr = (void*)d->d_un.d_ptr;
        const char * str = strtab_addr ? strtab_addr + d->d_un.d_ptr : "∅";
        switch (d->d_tag) {
        case DT_NEEDED:c->out->write("  {#bright_yellow}Needed{/} {#bold}{}{/}\n", str); break;
        case DT_PLTRELSZ: c->out->write("  {#bright_cyan}PLT relocs{/} #{#bold}{}{/}\n", d->d_un.d_val); break;
        case DT_PLTGOT: c->out->write("  {#bright_cyan}PLT/GOT address{/} @0x{:016X}\n", (void*)addr); break;
        case DT_HASH: c->out->write("  {#bright_cyan}Symbol hash table{/} @0x{:016X}\n", (void*)addr); break;
        case DT_STRTAB: c->out->write("  {#bright_cyan}String table{/} @0x{:016X}\n", (void*)addr); break;
        case DT_SYMTAB: c->out->write("  {#bright_magenta}Symbol table{/} @0x{:016X}\n", (void*)addr); break;
        case DT_RELA: c->out->write("  {#bright_magenta}RELA relocs{/} @0x{:016X}\n", (void*)addr); break;
        case DT_RELASZ: c->out->write("  {#bright_magenta}RELA relocs{/} #{}\n", d->d_un.d_val); break;
        case DT_RELAENT: c->out->write("  {#bright_magenta}sizeof(RELA reloc){/} {} bytes\n", d->d_un.d_val); break;
        case DT_STRSZ: c->out->write("  {#bright_cyan}sizeof(string table){/} {} bytes\n", d->d_un.d_val); break;
        case DT_SYMENT: c->out->write("  {#bright_magenta}sizeof(symbol table entry){/} {} bytes\n", d->d_un.d_val); break;
        case DT_INIT: c->out->write("  {#bright_blue,bold}Init function{/} @0x{:016X}\n", (void*)addr); break;
        case DT_FINI: c->out->write("  {#bright_red,bold}Fini function{/} @0x{:016X}\n", (void*)addr); break;
        case DT_SONAME: c->out->write("  {#bold}Shared object name (soname){/} {}\n", str); break;
        case DT_RPATH: c->out->write("  {#red,bold}RPATH{/} {}\n", str); break;
        case DT_SYMBOLIC: c->out->write("  {#bright_magenta}Sym search starat {/} @0x{:016X}\n", (void*)addr); break;
        case DT_REL: c->out->write("  {#magenta}REL relocs{/} @0x{:016X}\n", (void*)addr); break;
        case DT_RELSZ: c->out->write("  {#magenta}REL relocs{/} #{}\n", d->d_un.d_val); break;
        case DT_RELENT: c->out->write("  {#magenta}sizeof(REL reloc){/} {} bytes\n", d->d_un.d_val); break;
        case DT_PLTREL:c->out->write("  {#bright_magenta}PLT type{/} {}\n", d->d_un.d_val); break;
        case DT_DEBUG: c->out->write("  {#bright_red}Debug{/} {:016X}\n", d->d_un.d_val); break;
        case DT_TEXTREL: c->out->write("  {#bright_red}Relocs modify text{/} {}\n", d->d_un.d_val); break;
        case DT_JMPREL: c->out->write("  {#bright_magenta}PLT relocs{/} @0x{:016X}\n", (void*)addr); break;
        case DT_BIND_NOW: c->out->write("  {#bright_green}Current relocs (BIND_NOW){/} @0x{:016X}\n", (void*)addr); break;
        case DT_INIT_ARRAY: c->out->write("  {#bright_green}Init array{/} @0x{:016X}\n", (void*)addr); break;
        case DT_FINI_ARRAY: c->out->write("  {#bright_red}Fini array{/} @0x{:016X}\n", (void*)addr); break;
        case DT_INIT_ARRAYSZ:c->out->write("  {#bright_green}Init array size{/} #{} bytes\n", d->d_un.d_val); break;
        case DT_FINI_ARRAYSZ: c->out->write("  {#bright_red}Fini array size{/} #{} bytes\n", d->d_un.d_val); break;
        case DT_RUNPATH:c->out->write("  {#bright_yellow}Library search path{/} #{}\n", str); break;
        case DT_FLAGS:c->out->write("  {#bright_magenta}Flags{/} {:08X}\n", d->d_un.d_val); break;
        case DT_PREINIT_ARRAY: c->out->write("  {#bright_green,bold}PreInit array{/} @0x{:016X}\n", (void*)addr); break;
        case DT_PREINIT_ARRAYSZ:c->out->write("  {#bright_green,bold}PreInit array size{/} #{} bytes\n", d->d_un.d_val); break;
        case DT_SYMTAB_SHNDX: c->out->write("  {#bright_magenta}SYMTAB_SHNDX{/} @0x{:016X}\n", (void*)addr); break;
        case DT_RELCOUNT: c->out->write("  {#bright_magenta}RELCOUNT{/} #{}\n", d->d_un.d_val); break;
        case DT_RELACOUNT: c->out->write("  {#magenta}RELACOUNT{/} #{}\n", d->d_un.d_val); break;
        case DT_FLAGS_1: c->out->write("  {#bright_red}Flags {#bold}Linux{/}{/} {:08X}\n", d->d_un.d_val); break;
        case DT_VERNEED: c->out->write("  {#bright_yellow}Version dep table{/} @{:08X}\n", (void*)addr); break;
        case DT_VERNEEDNUM: c->out->write("  {#bright_yellow}Version dep table sz{/} #{}\n", d->d_un.d_val); break;
        default:
          if (d->d_tag >= DT_LOOS && d->d_tag <= DT_HIOS) {
            c->out->write("  {#bright_yellow,bold}OS-specific 0x{:08X}{/}\n", d->d_tag);
          } else if (d->d_tag >= DT_LOOS  && d->d_tag <= 0x6FFFFFFF) {
            c->out->write("  {#bright_yellow,bold}OS-specific, OLD 0x{:08X}{/}\n", d->d_tag);
          } else if (d->d_tag >= PT_LOPROC && d->d_tag <= PT_HIPROC) {
            c->out->write("  {#bright_red,bold}CPU-specific 0x{:08X}{/}\n", d->d_tag);
          } else {
            c->out->write("  {#error}Unknown #{:08X}{/}\n", d->d_tag);
          }
          break;
}

      }



      return 0;
    }


    void dump_phdr(mem::shared_ptr<strings::formatted_sink> out) {
      ctx c{static_cast<mem::shared_ptr<strings::formatted_sink> &&>(out)};
      dl_iterate_phdr(&iter_cb, &c);
    }

    namespace s = services;
    namespace c = cli;

    const s::service_instance<c::command> cmd{
      "mics.dump-phdr",
      "Dump PHDR command",
      c::command_name = "dump-phdr",
      c::callback = s::call(&dump_phdr, s::service("stdout")),
    };
  }
}
