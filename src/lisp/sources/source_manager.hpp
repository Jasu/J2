#pragma once

#include "containers/vector.hpp"
#include "lisp/sources/include_dirs.hpp"

namespace j::lisp::sources {
  struct source;
  struct string_source;
}

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::lisp::sources::source *);

namespace j::lisp::env {
  struct environment;
}

namespace j::lisp::sources {
  struct source_manager final {
    J_BOILERPLATE(source_manager, CTOR_CE)

    explicit source_manager(env::environment * J_NOT_NULL e, const env::compilation_options * J_NOT_NULL opts);

    ~source_manager();

    J_RETURNS_NONNULL string_source * add_dynamic_source(strings::const_string_view name);

    J_RETURNS_NONNULL source * add_static_source(strings::const_string_view name,
                                                                   strings::const_string_view code);

    J_RETURNS_NONNULL source * add_file_source(const files::path & path);

    [[nodiscard]] source * add_package_source(strings::const_string_view name);

    noncopyable_vector<j::lisp::sources::source *> sources;
    include_dir_set include_dirs;

    env::environment * environment = nullptr;
  };
}
