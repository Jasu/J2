#include "signals/signal.hpp"

namespace j::signals::detail {
  link & link::operator=(link && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      unbind();
      move_from(rhs);
    }
    return *this;
  }

  link * link::find_predecessor() const noexcept {
    link *cur = const_cast<link*>(this);
    for (;;) {
      link *next = cur->next();
      if (next == this) {
        return cur;
      }
      cur = next;
    }
  }

  void link::unbind() noexcept
  { find_predecessor()->set_next(next()); }
}
