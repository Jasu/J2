#pragma once

#include "state.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "functions/bound_function.hpp"
#include "mem/rw_buffer.hpp"

namespace j::lisp::sources {
  struct string_source;
}

namespace j::lisp::reader {
  enum class interactive_read_status : u8_t {
    finished,
    pending,
  };

  struct interactive_reader final {
    J_BOILERPLATE(interactive_reader, COPY_DEL, MOVE_DEL, CTOR_DEL)

    interactive_reader(env::environment * J_NOT_NULL env,
                       packages::pkg * J_NOT_NULL pkg,
                       j::functions::bound_function<void (lisp_imm)> callback);

    interactive_read_status read(sources::string_source * J_NOT_NULL src);
    interactive_read_status finalize();

    void on_top_level();

    j::mem::rw_buffer input_buffer;
    state state;
    j::functions::bound_function<void (lisp_imm)> callback;
  };
}
