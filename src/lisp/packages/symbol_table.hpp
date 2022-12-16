#pragma once

#include "lisp/symbols/symbol.hpp"
#include "lisp/packages/package_import_table.hpp"
#include "lisp/common/id_hash.hpp"
#include "lisp/env/name_table.hpp"

J_DECLARE_EXTERN_HASH_MAP(j::lisp::id, j::lisp::symbols::symbol*, HASH(j::lisp::id_hash));

namespace j::mem {
  class bump_pool;
}

namespace j::lisp::packages {
  struct symbol_table final {
    using map_t = hash_map<id, symbols::symbol*, id_hash>;

    symbol_table(u8_t package_id, j::mem::bump_pool * J_NOT_NULL sym_pool) noexcept;

    symbol_table(const symbol_table &) = delete;
    symbol_table & operator=(const symbol_table &) = delete;

    [[nodiscard]] J_RETURNS_NONNULL symbols::symbol * at(id name) noexcept;

    J_INLINE_GETTER id try_resolve(id name) noexcept {
      return name.is_unresolved() ? do_try_resolve(name) : name;
    }

    J_RETURNS_NONNULL symbols::symbol * resolve_unresolved(id name);

    [[nodiscard]] symbols::symbol * maybe_at(id name) const noexcept;

    void add_pend(id name, compilation::tlf * pend);

    J_RETURNS_NONNULL symbols::symbol * declare(id name, symbols::symbol_flags flags);

    J_RETURNS_NONNULL symbols::symbol * declare(id name, lisp_imm ast, symbols::symbol_flags flags, value_info::value_info info);

    J_ALWAYS_INLINE void add_package_import(id local_name, pkg * J_NOT_NULL pkg) {
      package_imports.add_package_import(local_name, pkg);
    }

    J_RETURNS_NONNULL symbols::symbol * add_import(id local_name, symbols::symbol * J_NOT_NULL sym);

    u32_t size() const noexcept;

    map_t map;

    package_import_table package_imports;

    /// Names of locally defined symbols, including locals.
    env::name_table names;
    j::mem::bump_pool * sym_pool = nullptr;

    [[nodiscard]] id do_try_resolve(id name) noexcept;
  };
}
