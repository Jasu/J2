#include "lisp/packages/symbol_table.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "containers/hash_map.hpp"
#include "exceptions/assert.hpp"
#include "mem/bump_pool.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::lisp::id, j::lisp::symbols::symbol*, HASH(j::lisp::id_hash));

namespace j::lisp::packages {

  symbol_table::symbol_table(u8_t package_id, j::mem::bump_pool * J_NOT_NULL sym_pool) noexcept
    : names(package_id),
      sym_pool(sym_pool)
  { }

  [[nodiscard]] J_RETURNS_NONNULL symbols::symbol * symbol_table::at(id name) noexcept {
    J_ASSERT(name && !name.is_gensym());
    if (symbols::symbol * const * const result = map.maybe_at(name)) {
      if (!(*result)->is_pending()) {
        return *result;
      }
    }
    return resolve_unresolved(name);
  }

  void symbol_table::add_pend(id name, compilation::tlf * pend) {
    J_ASSERT(name && !name.is_unresolved() && !name.is_gensym());
    if (auto previous = map.maybe_at(name)) {
      J_ASSERT((*previous)->name == name);
      J_ASSERT((*previous)->is_pending(), "{} was not pending.", name);
      (*previous)->set_pending(pend);
      return;
    }
    symbols::symbol * const result = &sym_pool->emplace<symbols::symbol>();
    *result = {
      lisp_imm{(u64_t)pend},
      symbols::symbol_state::pending,
      {},
      name,
      {},
    };
    map.emplace(name, result);
  }

  J_RETURNS_NONNULL symbols::symbol * symbol_table::declare(id name, symbols::symbol_flags flags)
  {
    J_ASSERT(name && !name.is_unresolved() && !name.is_gensym());
    auto previous = map.maybe_at(name);
    if (previous) {
      J_ASSERT((*previous)->name == name);
      J_ASSERT((*previous)->is_pending(), "Double-declared {}", name);
      (*previous)->notify_pending();
    }

    symbols::symbol * const result = previous ? *previous : &sym_pool->emplace<symbols::symbol>();
    *result = {
      lisp_nil{},
      flags.has(symbols::symbol_flag::initialized)
        ? symbols::symbol_state::initialized
        : symbols::symbol_state::uninitialized,
      flags,
      name,
      {},
    };
    if (!previous) {
      map.emplace(name, result);
    }
    return result;
  }

  J_RETURNS_NONNULL symbols::symbol * symbol_table::declare(id name, lisp_imm ast, symbols::symbol_flags flags, value_info::value_info info) {
    J_ASSERT(name && !name.is_unresolved() && !name.is_gensym());
    auto previous = map.maybe_at(name);
    if (previous) {
      J_ASSERT((*previous)->is_pending());
      (*previous)->notify_pending();
    }
    symbols::symbol * const result = previous ? *previous : &sym_pool->emplace<symbols::symbol>();
    *result = {
      ast,
      symbols::symbol_state::uninitialized,
      flags,
      name,
      info,
    };
    if (!previous) {
      map.emplace(name, result);
    }
    return result;
  }

  J_RETURNS_NONNULL symbols::symbol * symbol_table::resolve_unresolved(id name) {
    J_REQUIRE(name.is_unresolved() && !name.is_gensym(), "Tried to resolve an already resolved symbols::symbol {}", name);
    auto p = names.get_unresolved_ids(name);
    auto pkg = package_imports.at(p.pkg_id);
    auto symbol_name = names.name_of(p.sym_id);
    auto resolved = pkg->id_of(symbol_name->value());
    auto sym = pkg->symbol_table.at(resolved);
    add_import(name, sym);
    return sym;
  }

  id symbol_table::do_try_resolve(id name) noexcept {
    J_REQUIRE(name.is_unresolved() && !name.is_gensym(), "Tried to resolve an already resolved symbols::symbol {}", name);
    auto p = names.get_unresolved_ids(name);
    if (auto pkg = package_imports.maybe_at(p.pkg_id)) {
      auto symbol_name = names.name_of(p.sym_id);
      return pkg->symbol_table.names.try_get_id_of(symbol_name->value());
    }
    return id::none;
  }

  J_RETURNS_NONNULL symbols::symbol * symbol_table::add_import(id local_name, symbols::symbol * J_NOT_NULL sym) {
    J_ASSERT(local_name && !local_name.is_unresolved() && !local_name.is_gensym());
    J_ASSERT(sym->is_exported());
    [[maybe_unused]] auto did_insert = map.emplace(local_name, sym).second;
    J_ASSERT(did_insert);
    return sym;
  }
  u32_t symbol_table::size() const noexcept {
    return map.size();
  }
  symbols::symbol * symbol_table::maybe_at(id name) const noexcept {
    if (symbols::symbol * const * const result = map.maybe_at(name)) {
      if ((*result)->is_pending()) {
        return nullptr;
      }
      return *result;
    }
    return nullptr;
  }
}
