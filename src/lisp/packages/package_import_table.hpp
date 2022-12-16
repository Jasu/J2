#pragma once

#include "lisp/common/id_hash.hpp"
#include "containers/hash_map_fwd.hpp"

namespace j::lisp::compilation {
  struct tlf;
}

namespace j::lisp::packages {
  struct pkg_import_record final {
    J_BOILERPLATE(pkg_import_record, CTOR_CE)

    J_ALWAYS_INLINE pkg_import_record(struct pkg * J_NOT_NULL package) noexcept
    : package(package)
    { }

    J_ALWAYS_INLINE pkg_import_record(compilation::tlf * J_NOT_NULL pending) noexcept
    : first_pending(pending)
    { }

    struct pkg * package = nullptr;
    compilation::tlf * first_pending = nullptr;
  };
}

J_DECLARE_EXTERN_HASH_MAP(j::lisp::id, j::lisp::packages::pkg_import_record, HASH(j::lisp::id_hash));

namespace j::lisp::packages {

  struct package_import_table final {
    package_import_table() noexcept;

    [[nodiscard]] J_RETURNS_NONNULL pkg * at(id local_name) const noexcept;

    [[nodiscard]] pkg * maybe_at(id local_name) const noexcept;

    void add_pend(id local_name, compilation::tlf * J_NOT_NULL pendee);

    void add_package_import(id local_name, pkg * J_NOT_NULL pkg);

    hash_map<id, j::lisp::packages::pkg_import_record, id_hash> map;
  };
}
