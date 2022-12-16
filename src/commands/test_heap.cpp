#include "cli/cli.hpp"
#include "lisp/lisp_fwd.hpp"
#include "lisp/mem/heap.hpp"
#include "lisp/mem/heap_cell.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/sources/source_location.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::commands {
  namespace {
    namespace l = j::lisp;
    namespace str = j::strings;

    void test_heap(mem::shared_ptr<str::formatted_sink> stdout) {
      l::mem::heap heap;
      stdout->write("{#bold}Empty{/}");
      heap.dump();
      stdout->write("\n{#bold}Add AA{/}\n");
      (void)l::lisp_str::allocate(heap, "AA");
      heap.dump();
      stdout->write("\n{#bold}Add A{/}\n");
      l::lisp_str * mid = l::lisp_str::allocate(heap, "A");
      heap.dump();
      stdout->write("\n{#bold}Add Froobafoo{/}\n");
      auto str = l::lisp_str::allocate(heap, "Froobafoo");
      stdout->write("\n\n{}\n", (void*)str, *str);
      heap.dump();
      stdout->write("\n{#bold}Add (1 2 3){/}\n");
      l::lisp_imm imms[3] = {
        l::lisp_i64(1),
        l::lisp_i64(2),
        l::lisp_i64(3),
      };
      l::lisp_vec::allocate_with_debug_info(heap, l::sources::source_location{1,1232,121}, imms);
      heap.dump();
      stdout->write("\n{#bold}Freed A{/}\n");
      heap.release(mid);
      heap.dump();
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "commands.test_heap",
      "Test Lisp heap",

      c::command_name = "test-heap",
      c::callback     = s::call(&test_heap,
                                s::service("stdout")));
  }
}
