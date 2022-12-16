#pragma once

#include "files/dirs/dir.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::lisp::env {
  struct compilation_options;
}

namespace j::lisp::sources {
  struct include_dir final {
    J_BOILERPLATE(include_dir, CTOR_CE, MOVE_NE)

    u32_t index = 0U;
    files::dirs::dir dir;

    include_dir(u32_t i, const files::path & p);
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::lisp::sources::include_dir);

namespace j::lisp::sources {
  struct include_dir_set final {
    J_BOILERPLATE(include_dir_set, CTOR_CE, MOVE_NE)

    explicit include_dir_set(const env::compilation_options * J_NOT_NULL opts);

    trivial_array<include_dir> dirs;

    J_INLINE_GETTER include_dir & operator[](u32_t i) noexcept {
      return dirs[i];
    }

    J_INLINE_GETTER const include_dir & operator[](u32_t i) const noexcept {
      return dirs[i];
    }

    [[nodiscard]] include_dir * find_dir_with_file(const files::path & path);
  };
}
