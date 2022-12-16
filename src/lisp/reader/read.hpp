#pragma once

#include "lisp/lisp_fwd.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::reader {
  struct state;
  struct interactive_reader;

  enum class read_status : u8_t {
    eof,
    waiting,
    top_level,
  };

  lisp_vec * read(env::environment * J_NOT_NULL env, packages::pkg * J_NOT_NULL pkg, sources::source * src);

  read_status read_interactive(interactive_reader & reader);
}
