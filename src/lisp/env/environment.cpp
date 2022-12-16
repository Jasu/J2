#include "lisp/env/environment.hpp"
#include "lisp/env/static_ids.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/symbols/symbol.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/air/air_package.hpp"
#include "lisp/air/passes/const_fold.hpp"
#include "containers/vector.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::lisp::packages::pkg*);

namespace j::lisp::env {
  namespace s = strings;
  environment::environment()
    : code_heap(J_PAGE_SIZE * 256, J_PAGE_SIZE * 256)
  {
    i32_t i = 0;
    for (auto * pkg_def : static_pkgs) {
      auto pkg = ::new packages::pkg(this, i);
      packages[i] = pkg;
      for (auto * str : *pkg_def) {
        pkg->emplace_id(str);
      }
      pkg->status = packages::pkg_status::compiled;
      ++i;
    }

    packages[package_id_air] = air::create_air_package(this);
    packages[package_id_const_fold] = air::create_const_fold_pkg(this);
  }

  environment::~environment() {
    for (packages::pkg * p : packages) {
      ::delete p;
    }
    ::delete m_env_compiler;
    ::delete air_const_fold_table;
  }

  J_A(RNN) packages::pkg * environment::new_package(strings::const_string_view name) {
    auto id = packages[package_id_packages]->emplace_id(name);
    J_ASSERT(id.index() < 256 && !packages[id.index()]);
    return packages[id.index()] = ::new packages::pkg(this, id.index());
  }

  J_A(RNN) packages::pkg * environment::get_package(const lisp_str * J_NOT_NULL name) {
    J_ASSERT(name->size());
    J_ASSUME_NOT_NULL(packages[package_id_packages]);
    u32_t index = packages[package_id_packages]->id_of(name->value()).index();
    J_ASSUME(index < 256);
    J_ASSUME_NOT_NULL(packages[index]);
    return packages[index];
  }

  [[nodiscard]] packages::pkg * environment::try_get_package(strings::const_string_view name) noexcept {
    J_ASSERT(name.size());
    J_ASSUME_NOT_NULL(packages[package_id_packages]);
    if (id index = packages[package_id_packages]->try_get_id_of(name)) {
      J_ASSERT(index.index() < 256);
      return packages[index.index()];
    }
    return nullptr;
  }

  J_A(RNN) env_compiler * environment::get_env_compiler(const compilation_options * J_NOT_NULL opts) {
    if (!m_env_compiler) {
      m_env_compiler = ::new env_compiler(opts, this);
    } else {
      m_env_compiler->opts = opts;
    }
    return m_env_compiler;
  }

  J_A(RNN,NODISC) packages::pkg* environment::package_at(s::const_string_view name) const noexcept {
    J_ASSUME_NOT_NULL(packages[package_id_packages]);
    id index = packages[package_id_packages]->id_of(name);
    J_ASSERT_NOT_NULL(index);
    packages::pkg* result = packages[index.index()];
    J_ASSUME_NOT_NULL(result);
    return result;
  }

  [[nodiscard]] const lisp_str & environment::package_name(u8_t package_id) const noexcept {
    return *packages[package_id_packages]->name_of(id(package_id_packages, package_id));
  }

  [[nodiscard]] u8_t environment::package_id(strings::const_string_view name) const noexcept {
    return packages[package_id_packages]->id_of(name).index();
  }

  [[nodiscard]] const lisp_str * environment::symbol_name(id id) const noexcept {
    return packages[id.package_id()]->name_of(id);
  }
}
