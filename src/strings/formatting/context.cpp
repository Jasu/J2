#include "strings/formatting/context.hpp"
#include "hzd/mem.hpp"

namespace j::strings::inline formatting {
  J_A(NODESTROY,ND) thread_local j::util::context_stack<formatting_context_t, true> formatting_context{formatting_context_t{}};
  formatting_context_t::formatting_context_t(const formatting_context_t * parent) noexcept
    : formatting_context_t(*parent)
  { }
}
