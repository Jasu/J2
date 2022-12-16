#pragma once

#include "lisp/mem/heap.hpp"
#include "lisp/mem/code_heap/code_heap.hpp"
#include "lisp/common/id.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::inline values {
  struct lisp_str;
}

namespace j::lisp::symbols {
  struct symbol;
}

namespace j::lisp::packages {
  struct pkg;
}

namespace j::lisp::air::inline passes {
  struct const_fold_table;
}

namespace j::lisp::env {
  struct env_compiler;
  struct compilation_options;

  /// Environment for both compiling and running code.
  struct environment final {
    J_BOILERPLATE(environment, COPY_DEL)
    environment();

    ~environment();

    J_A(RNN,NODISC) packages::pkg * get_package(const lisp_str * J_NOT_NULL name);

    J_A(RNN,NODISC) env_compiler * get_env_compiler(const compilation_options * J_NOT_NULL opts);

    J_A(NODISC,AI,ND,HIDDEN) inline env_compiler * maybe_get_env_compiler() noexcept {
      return m_env_compiler;
    }

    J_A(RNN) packages::pkg * new_package(strings::const_string_view name);

    [[nodiscard]] packages::pkg * try_get_package(strings::const_string_view name) noexcept;

    J_A(RNN) packages::pkg * get_or_create_package(strings::const_string_view name) {
      packages::pkg * result = try_get_package(name);
      return result ? result : new_package(name);
    }

    J_A(RNN,NODISC,AI,ND,HIDDEN) inline packages::pkg* package_at(u8_t package_id) const noexcept {
      return packages[package_id];
    }

    J_A(RNN,NODISC) packages::pkg* package_at(strings::const_string_view package_name) const noexcept;

    [[nodiscard]] const lisp_str & package_name(u8_t package_id) const noexcept;

    [[nodiscard]] u8_t package_id(strings::const_string_view name) const noexcept;

    [[nodiscard]] const lisp_str * symbol_name(id id) const noexcept;

    mem::heap heap;
    mem::code_heap::code_heap code_heap;
    packages::pkg* packages[256] = { nullptr };

    env_compiler * m_env_compiler = nullptr;
    const air::const_fold_table * air_const_fold_table = nullptr;
  };
}
