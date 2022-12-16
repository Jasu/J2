#include "symbol.hpp"
#include "lisp/compilation/source_compilation.hpp"

namespace j::lisp::symbols {
  void symbol::notify_pending() noexcept {
    if (is_pending()) {
      for (compilation::tlf * pender = (compilation::tlf*)value.raw;
           pender;)
      {
        auto next = pender->next_pending;
        pender->next_pending = *pender->work_list;
        *pender->work_list = pender;
        pender = next;
      }
      value.raw = 0U;
    }
  }

  void symbol::set_pending(compilation::tlf * J_NOT_NULL pender) noexcept {
    if (!is_pending()) {
      J_ASSERT(state == symbol_state::uninitialized);
      state = symbol_state::pending;
    }

    compilation::tlf * previous = (compilation::tlf*)value.raw;
    pender->next_pending = previous;
    value.raw = (u64_t)pender;
  }
}
