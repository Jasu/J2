#include "lisp/sources/source_manager.hpp"

#include "lisp/env/environment.hpp"
#include "lisp/values/lisp_str.hpp"
#include "lisp/sources/source.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::lisp::sources::source *);

namespace j::lisp::sources {
  source_manager::source_manager(env::environment * J_NOT_NULL e, const env::compilation_options * J_NOT_NULL opts)
    : include_dirs(opts),
      environment(e)
  {
  }

  source_manager::~source_manager() {
    for (auto * src : sources) {
      J_ASSUME_NOT_NULL(src);
      src->name->ext_unref();
      ::delete(src);
    }
  }

  string_source * source_manager::add_dynamic_source(strings::const_string_view name) {
    J_ASSERT_NOT_NULL(name, environment);
    auto src = ::new j::lisp::sources::string_source(
      false,
      sources.size(),
      lisp_str::allocate(environment->heap, name, (lisp::mem::object_hightag_flag)0, 1U),
      "");
    sources.push_back(src);
    return src;
  }

  source * source_manager::add_static_source(strings::const_string_view name,
                                                               strings::const_string_view code)
  {
    J_ASSERT_NOT_NULL(name, environment);
    auto src = ::new j::lisp::sources::string_source(
      false,
      sources.size(),
      lisp_str::allocate(environment->heap, name, (lisp::mem::object_hightag_flag)0, 1U),
      code);
    sources.push_back(src);
    return src;
  }

  j::lisp::sources::source * source_manager::add_file_source(const files::path & path)
  {
    J_ASSERT_NOT_NULL(environment);
    auto src = ::new j::lisp::sources::file_source(
      sources.size(),
      include_dirs.find_dir_with_file(path),
      lisp_str::allocate(environment->heap, path.as_c_string(), (lisp::mem::object_hightag_flag)0, 1U));
    sources.push_back(src);
    return src;
  }

  j::lisp::sources::source * source_manager::add_package_source(strings::const_string_view name) {
    J_ASSERT_NOT_NULL(name, environment);
    files::path pkg_dir_path{name + "/init.lisp"};
    files::path pkg_file_path{name + ".lisp"};
    include_dir * pkg_dir_dir = include_dirs.find_dir_with_file(pkg_dir_path);
    include_dir * pkg_file_dir = include_dirs.find_dir_with_file(pkg_file_path);
    if (!pkg_dir_dir && !pkg_file_dir) {
      return nullptr;
    }
    bool is_dir = false;
    if (pkg_dir_dir && pkg_file_dir) {
      is_dir = pkg_dir_dir->index < pkg_file_dir->index;
    } else if (pkg_dir_dir) {
      is_dir = true;
    }

    auto src = ::new j::lisp::sources::file_source(
      sources.size(),
      is_dir ? pkg_dir_dir : pkg_file_dir,
      lisp_str::allocate(environment->heap,
                         (is_dir ? pkg_dir_path : pkg_file_path).as_c_string(),
                         (lisp::mem::object_hightag_flag)0, 1U));
    sources.push_back(src);
    return src;
  }
}
