#include "lisp/assembly/functions/trap_handler.hpp"
#include "lisp/assembly/functions/func_info.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/env/context.hpp"
#include "logging/global.hpp"
#include "exceptions/exceptions.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <libunwind.h>

namespace j::lisp::assembly::inline functions {
  [[gnu::used]] void error_handler() { // trap_info * J_NOT_NULL trap, const char * J_NOT_NULL fn_name) {
    throw j::exceptions::logic_error_exception() << j::exceptions::message("Invalid type");
  }

  __attribute__((naked)) void trampoline(void) {
    asm("pop %rbp");
    asm("jmp *%0" : : "r"(error_handler));
  }

  namespace {
    void handle_trap(int signum, siginfo_t* info, void* ucontext) {
      if (signum != SIGILL) {
        _exit(128 + signum);
      }
      unw_cursor_t c;
      unw_init_local2(&c, (unw_context_t*)ucontext, UNW_INIT_SIGNAL_FRAME);
      unw_proc_info_t proc;
      unw_get_proc_info(&c, &proc);
      if (proc.handler != 0 || proc.format != UNW_INFO_FORMAT_DYNAMIC) {
        _exit(128 + signum);
      }
      void * fault_addr = info->si_addr;

      unw_word_t offset;
      char name_buffer[256];
      unw_get_proc_name(&c, name_buffer, 256, &offset);
      const char * name_ptr = strchr(name_buffer, ':');
      if (!name_ptr) {
        _exit(128 + signum);
      }

      auto pkg = env::env_context->env->try_get_package(strings::const_string_view(name_buffer, name_ptr));
      if (!pkg) {
        _exit(128 + signum);
      }

      name_ptr++;
      auto sym = pkg->symbol_table.maybe_at(pkg->id_of(strings::const_string_view(name_ptr)));
      if (!sym) {
        _exit(128 + signum);
      }

      auto func_info =  sym->value_info.func_info;
      if (!func_info) {
        _exit(128 + signum);
      }

      for (auto cc : lisp::functions::ccs) {
        void * fn_begin = func_info->cc_info[cc].fn_ptr;
        if (!fn_begin) {
          continue;
        }
        if (func_info->cc_info[cc].asm_info) {
          for (auto & trap : func_info->cc_info[cc].asm_info->traps()) {
            if (add_bytes(fn_begin, trap.offset) == fault_addr) {
              if (unw_set_reg(&c, UNW_REG_IP, (unw_word_t)&trampoline)) {
                J_FAIL("IP");
              }
              return;
            }
          }
        }
      }

      _exit(128 + signum);
    }
  }

  void install_trap_handler() noexcept {
    struct sigaction sa;
    sa.sa_sigaction = &handle_trap;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigaction(SIGILL, &sa, nullptr);
  }
}
