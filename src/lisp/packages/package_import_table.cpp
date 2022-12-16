#include "lisp/packages/package_import_table.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/compilation/source_compilation.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::lisp::id, j::lisp::packages::pkg_import_record, HASH(j::lisp::id_hash));

namespace j::lisp::packages {
  package_import_table::package_import_table() noexcept {
  }

  void package_import_table::add_package_import(id local_name, pkg * J_NOT_NULL pkg) {
    J_ASSERT(local_name && !local_name.is_unresolved() && !local_name.is_gensym());
    J_ASSERT(local_name.package_id() != pkg->symbol_table.names.package_id);
    auto p = map.emplace(local_name, pkg);
    if (p.second) {
      return;
    }
    if (p.first->second.package) {
      J_ASSERT(!p.first->second.first_pending);
      return;
    }
    J_ASSERT(p.first->second.first_pending);
    for (auto cur = p.first->second.first_pending; cur;) {
      J_ASSUME_NOT_NULL(cur->work_list);
      auto next = cur->next_pending;
      cur->next_pending = *cur->work_list;
      *cur->work_list = cur;
      cur = next;
    }
    p.first->second.first_pending = nullptr;
    p.first->second.package = pkg;
  }

  [[nodiscard]] J_RETURNS_NONNULL pkg * package_import_table::at(id local_name) const noexcept {
    J_ASSERT(local_name && !local_name.is_unresolved());
    auto & result = map.at(local_name);
    J_ASSERT_NOT_NULL(result.package, "Package is still pending.");
    return result.package;
  }

  [[nodiscard]] pkg * package_import_table::maybe_at(id local_name) const noexcept {
    J_ASSERT(local_name && !local_name.is_unresolved());
    auto * result = map.maybe_at(local_name);
    return result ? result->package : nullptr;
  }

  void package_import_table::add_pend(id local_name, compilation::tlf * J_NOT_NULL pendee) {
    J_ASSERT(local_name && !local_name.is_unresolved());
    auto p = map.emplace(local_name, pendee);
    if (p.second) {
      // No need to set manually, was first to emplace.
      return;
    }
    J_ASSERT(p.first->second.package == nullptr);
    J_ASSERT(pendee->next_pending == nullptr && pendee->work_list);
    pendee->next_pending = p.first->second.first_pending;
    p.first->second.first_pending = pendee;
  }
}
