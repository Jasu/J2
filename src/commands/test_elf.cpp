#include "cli/cli.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "os/dl.hpp"
#include "os/dwarf_reader.hpp"

namespace j::commands {
  namespace {
    namespace s = j::strings;
    namespace d = os::dwarf;
    void test_elf(mem::shared_ptr<s::formatted_sink> out) {
      os::loaded_library self = os::get_loaded_library("libj.so");
      if (!self) {
        out->write("Could not find self (libj.so) in current executable.");
        return;
      }
      out->write("Found self in memory libraries:\n"
                 "     soname: {}\n"
                 "       path: {}\n"
                 "  loaded at: {}\n",
                 self.soname, self.path, (void*)self.loaded_at);

      d::dwarf_reader r(self.path);
      const d::dwarf_info info = r.read();
    }

    namespace c = cli;
    namespace svc = services;
    J_A(ND,NODESTROY) const svc::service_instance<c::command> cmd(
      "commands.test_elf",
      "Test ELF parsing",

      c::command_name = "test-elf",
      c::callback     = svc::call(&test_elf, svc::service("stdout")));
  }
}
