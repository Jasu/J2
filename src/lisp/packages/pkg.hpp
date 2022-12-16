#pragma once

#include "lisp/packages/symbol_table.hpp"
#include "lisp/lisp_fwd.hpp"
#include "lisp/common/id.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "lisp/functions/func_wrapper.hpp"
#include "mem/bump_pool.hpp"

namespace j::lisp::compilation {
  struct pkg_context;
}

namespace j::lisp::packages {
  enum class pkg_status : u8_t {
    compiling,
    compiled,
    loading,
    loaded,
  };

  /// A package.
  ///
  /// From other packages' perspective, a package is a named collection of exported symbols that may
  /// refer to functions, macros, or values.
  ///
  /// From the execution environment's perspective, packages are also a collection of functions,
  /// values, imports, and exports along with the compilation state for each of them.
  struct pkg final {
    pkg(env::environment * J_NOT_NULL env, u8_t package_id) noexcept;

    [[nodiscard]] const lisp_str * name_of(id id) const;

    [[nodiscard]] id id_of(const strings::const_string_view & name);

    id emplace_id(const lisp_str * J_NOT_NULL name) noexcept;

    id emplace_id(const lisp_str_ref & name) noexcept;

    id emplace_id(strings::const_string_view name) noexcept;

    [[nodiscard]] id try_get_id_of(strings::const_string_view str) const noexcept;

    functions::func_wrapper get_fn(id id);
    functions::func_wrapper get_fn(const strings::const_string_view & name);
    functions::func_wrapper get_fn(const lisp_str * J_NOT_NULL name);

    void load();

    id emplace_unresolved_id(const strings::const_string_view & package_name, const strings::const_string_view & symbol_name);

    void register_foreign_function(id name, lisp_fn_ptr_t J_NOT_NULL fn, void * abi_call = nullptr);
    void register_foreign_function(const lisp_str * J_NOT_NULL name, lisp_fn_ptr_t J_NOT_NULL fn, void * abi_call = nullptr);
    void register_foreign_function(const strings::const_string_view & name, lisp_fn_ptr_t J_NOT_NULL fn, void * abi_call = nullptr);

    [[nodiscard]] id gensym() noexcept;

    J_RETURNS_NONNULL symbols::symbol * register_symbol(id name, lisp_imm value, symbols::symbol_flags flags = {});
    J_RETURNS_NONNULL symbols::symbol * register_symbol(const lisp_str * J_NOT_NULL name, lisp_imm value, symbols::symbol_flags flags = {});
    J_RETURNS_NONNULL symbols::symbol * register_symbol(const strings::const_string_view & name, lisp_imm value, symbols::symbol_flags flags = {});

    J_RETURNS_NONNULL symbols::symbol * register_const(id name, lisp_imm value, bool is_exported = true);
    J_RETURNS_NONNULL symbols::symbol * register_const(const lisp_str * J_NOT_NULL name, lisp_imm value, bool is_exported = true);
    J_RETURNS_NONNULL symbols::symbol * register_const(strings::const_string_view name, lisp_imm value, bool is_exported = true);

    J_RETURNS_NONNULL symbols::symbol * register_var(id name, lisp_imm value, bool is_exported = true);
    J_RETURNS_NONNULL symbols::symbol * register_var(const lisp_str * J_NOT_NULL name, lisp_imm value, bool is_exported = true);
    J_RETURNS_NONNULL symbols::symbol * register_var(const strings::const_string_view & name, lisp_imm value, bool is_exported = true);

    lisp_imm eval(sources::source * J_NOT_NULL src);
    lisp_imm eval(lisp_imm imm, sources::source * J_NOT_NULL src);

    compilation::source_compilation_context & get_eval_compiler(sources::source * J_NOT_NULL src);

    J_RETURNS_NONNULL symbols::symbol * compile_eval(sources::source * J_NOT_NULL src);

    J_RETURNS_NONNULL symbols::symbol * create_load_symbol();
    J_RETURNS_NONNULL symbols::symbol * create_eval_symbol();
    void clear_eval_symbol();
    J_RETURNS_NONNULL symbols::symbol * create_hidden_function_symbol(symbols::symbol ** J_NOT_NULL sym, strings::const_string_view name);
    void clear_hidden_function_symbol(symbols::symbol * sym);

    compilation::pkg_context & get_pkg_context();

    /// Values of locally defined and imported symbols, excluding locals
    symbol_table symbol_table;
    pkg_status status = pkg_status::compiling;
    /// Pool for symbols and their metadata.
    j::mem::bump_pool metadata_pool;
    /// Environment of the package.
    env::environment * env = nullptr;
    /// Load function of the package.
    symbols::symbol * load_symbol = nullptr;
    /// Eval-compiled temp function of the package.
    symbols::symbol * eval_symbol = nullptr;
    /// Number of the current gensym.
    u32_t gensym_counter = 0U;

    compilation::pkg_context * cur_context = nullptr;
  };
}
