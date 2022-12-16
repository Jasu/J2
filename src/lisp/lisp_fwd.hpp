#pragma once

#include "strings/string.hpp"
#include "containers/span.hpp"

namespace j::strings {
  class string;
  template<typename> class basic_string_view;
  using string_view = basic_string_view<char>;
  using const_string_view = basic_string_view<const char>;
}

namespace j::lisp::inline common {
  struct metadata;
  struct metadata_view;
  struct metadata_init;
  using metadata_inits_t = span<metadata_init>;
  using const_metadata_inits_t = span<const metadata_init>;
}

namespace j::lisp {
  namespace compilation {
    struct context;
  }
  namespace env {
    struct environment;
    struct env_compiler;
    struct compilation_options;
  }
  namespace cir::inline ssa {
    struct cir_ssa;
  }

  inline namespace values {
    struct lisp_object;
    struct lisp_str;
    struct lisp_vec;

    struct lisp_imm;
    struct lisp_i64;
    struct lisp_f32;
    struct lisp_nil;
    struct lisp_bool;

    struct lisp_vec_ref;
    struct lisp_rec_ref;
    struct lisp_str_ref;
    struct lisp_sym_id;
  }
  namespace compilation {
    struct compilation_context;
    struct source_compilation_context;
    struct root_source_compilation_context;
  }
  namespace packages {
    struct pkg;
  }
  namespace symbols {
    struct symbol;
  }
  namespace sources {
    struct source;
  }
  namespace reader {
    lisp_vec * read(env::environment * env, packages::pkg * J_NOT_NULL pkg, sources::source * src);
  }
  namespace functions {
    struct func_info;
  }
  namespace mem::code_heap {
    struct allocation;
  }

  namespace assembly::inline linking {
    struct reloc;
  }
  namespace assembly::inline functions {
    struct function;
  }
  namespace cir_to_mc {
    struct mc_function;
  }

  using env::environment;
  using reader::read;
}
