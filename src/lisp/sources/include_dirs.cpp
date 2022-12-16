#include "lisp/sources/include_dirs.hpp"

#include "lisp/env/compilation_options.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::lisp::sources::include_dir);

namespace j::lisp::sources {
  include_dir::include_dir(u32_t i, const files::path & p)
    : index(i),
      dir(p)
  { }

  include_dir_set::include_dir_set(const env::compilation_options * J_NOT_NULL opts)
    : dirs(containers::uninitialized, opts->include_paths.size())
  {
    u32_t i = 0U;
    for (auto & p : opts->include_paths) {
      dirs.initialize_element(++i, p);
    }
  }
  [[nodiscard]] include_dir * include_dir_set::find_dir_with_file(const files::path & path) {
    J_ASSERT_NOT_NULL(path);
    for (auto & p : dirs) {
      if (p.dir.file_exists_at(path)) {
        return &p;
      }
    }
    return nullptr;
  }
}
