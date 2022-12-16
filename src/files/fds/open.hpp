#pragma once

#include "files/open_flags.hpp"
#include "strings/string_view.hpp"
#include "files/fds/fd.hpp"

namespace j::files::inline paths {
  class path;
}
namespace j::files::inline fds {
  struct fd;
  J_A(NODISC) fd open(strings::const_string_view path, open_flags_t flags);

  J_A(AI,NODISC,HIDDEN,ND) inline fd open(const char * J_NOT_NULL path, open_flags_t flags) {
    return open(strings::const_string_view(path), flags);
  }

  /// Open a path as an FD object.
  J_A(NODISC) fd open(const path & path, open_flags_t flags);

  J_A(NODISC) fd open_at(const fd & at, const path & path, open_flags_t flags);
}
