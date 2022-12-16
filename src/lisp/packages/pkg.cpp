#include "lisp/packages/pkg.hpp"

#include "lisp/env/environment.hpp"

#include "containers/vector.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/compilation/pkg_context.hpp"
#include "lisp/functions/func_info.hpp"
#include "lisp/compilation/source_compilation.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/compilation/compile_defun.hpp"
#include "lisp/compilation/compile_symbol.hpp"

namespace j::lisp::packages {
  pkg::pkg(env::environment * J_NOT_NULL env, u8_t package_id) noexcept
    : symbol_table(package_id, &metadata_pool),
      env(env)
  { }

  id pkg::emplace_id(strings::const_string_view name) noexcept{
    J_ASSERT(name);
    return symbol_table.names.emplace(name, &env->heap);
  }

  id pkg::emplace_id(const lisp_str * J_NOT_NULL name) noexcept {
    return symbol_table.names.emplace(name);
  }

  id pkg::emplace_id(const lisp_str_ref & name) noexcept {
    return symbol_table.names.emplace(name);
  }

  [[nodiscard]] id pkg::try_get_id_of(strings::const_string_view name) const noexcept {
    return symbol_table.names.try_get_id_of(name);
  }

  const lisp_str * pkg::name_of(id id) const {
    return symbol_table.names.name_of(id);
  }

  id pkg::id_of(const strings::const_string_view & name) {
    return symbol_table.names.id_of(name);
  }

  id pkg::emplace_unresolved_id(const strings::const_string_view & package_name, const strings::const_string_view & symbol_name) {
    J_ASSERT(package_name && symbol_name);
    return symbol_table.names.emplace_unresolved(package_name, symbol_name, &env->heap);
  }

  void pkg::register_foreign_function(id name, lisp_fn_ptr_t J_NOT_NULL fn, void * abi_call) {
    J_ASSERT(name && !name.is_gensym() && !name.is_unresolved());
    symbols::symbol * sym = symbol_table.declare(name, {symbols::symbol_flag::constant, symbols::symbol_flag::initialized, symbols::symbol_flag::exported});
    sym->value = lisp_fn_ref(fn);
    sym->value_info = value_info::value_info(imm_fn_ref, value_info::foreign_value_flags);
    auto info = &metadata_pool.emplace<functions::func_info>();
    info->cc_info[functions::calling_convention::full_call].fn_ptr = (void*)fn;
    info->cc_info[functions::calling_convention::abi_call].fn_ptr = abi_call;
    sym->value_info.func_info = info;
  }

  void pkg::register_foreign_function(const lisp_str * J_NOT_NULL name, lisp_fn_ptr_t J_NOT_NULL fn, void * abi_call) {
    register_foreign_function(symbol_table.names.emplace(name), fn, abi_call);
  }

  void pkg::register_foreign_function(const strings::const_string_view & name, lisp_fn_ptr_t J_NOT_NULL fn, void * abi_call) {
    register_foreign_function(emplace_id(name), fn, abi_call);
  }

  J_RETURNS_NONNULL symbols::symbol * pkg::register_symbol(id name, lisp_imm value, symbols::symbol_flags flags) {
    J_ASSERT(name && !name.is_gensym() && !name.is_unresolved());
    flags.set(symbols::symbol_flag::initialized);
    symbols::symbol * sym = symbol_table.declare(name, flags);
    sym->value = value;
    return sym;
  }

  J_RETURNS_NONNULL symbols::symbol * pkg::register_symbol(const lisp_str * J_NOT_NULL name, lisp_imm value, symbols::symbol_flags flags) {
    return register_symbol(symbol_table.names.emplace(name), value, flags);
  }
  J_RETURNS_NONNULL symbols::symbol * pkg::register_symbol(const strings::const_string_view & name, lisp_imm value, symbols::symbol_flags flags) {
    return register_symbol(emplace_id(name), value, flags);
  }

    J_RETURNS_NONNULL symbols::symbol * pkg::register_const(id name, lisp_imm value, bool is_exported) {
      auto sym = register_symbol(name, value, is_exported ? symbols::symbol_flag::constant | symbols::symbol_flag::exported : symbols::symbol_flag::constant);
      sym->value_info.types = value.type();
      return sym;
    }
    J_RETURNS_NONNULL symbols::symbol * pkg::register_const(const lisp_str * J_NOT_NULL name, lisp_imm value, bool is_exported) {
      auto sym = register_symbol(name, value, is_exported ? symbols::symbol_flag::constant | symbols::symbol_flag::exported : symbols::symbol_flag::constant);
      sym->value_info.types = value.type();
      return sym;
    }
    J_RETURNS_NONNULL symbols::symbol * pkg::register_const(strings::const_string_view name, lisp_imm value, bool is_exported) {
      auto sym = register_symbol(name, value, is_exported ? symbols::symbol_flag::constant | symbols::symbol_flag::exported : symbols::symbol_flag::constant);
      sym->value_info.types = value.type();
      return sym;
    }

    J_RETURNS_NONNULL symbols::symbol * pkg::register_var(id name, lisp_imm value, bool is_exported) {
      return register_symbol(name, value, is_exported ? symbols::symbol_flag::exported : symbols::symbol_flags{});
    }
    J_RETURNS_NONNULL symbols::symbol * pkg::register_var(const lisp_str * J_NOT_NULL name, lisp_imm value, bool is_exported) {
      return register_symbol(name, value, is_exported ? symbols::symbol_flag::exported : symbols::symbol_flags{});
    }
    J_RETURNS_NONNULL symbols::symbol * pkg::register_var(const strings::const_string_view & name, lisp_imm value, bool is_exported) {
      return register_symbol(name, value, is_exported ? symbols::symbol_flag::exported : symbols::symbol_flags{});
    }

  void pkg::load() {
    if (status == pkg_status::loaded) {
      return;
    }
    J_REQUIRE(status != pkg_status::loading, "Tried to load a package by dependency loop.");
    J_REQUIRE(status == pkg_status::compiled, "Tried to load a still compiling package.");
    status = pkg_status::loading;
    for (auto & pair : symbol_table.package_imports.map) {
      if (!pair.second.package) {
        J_FAIL("Oh no");
      }
      pair.second.package->load();
    }
    if (load_symbol && load_symbol->value_info.is_function()) {
      load_symbol->value.as_fn_ref().value()(nullptr, 0U);
    }
    status = pkg_status::loaded;
  }

  compilation::pkg_context & pkg::get_pkg_context() {
    if (!cur_context) {
      auto comp = env->maybe_get_env_compiler();
      J_ASSERT(comp, "TODO");
      cur_context = ::new compilation::pkg_context(*comp, *this);
    }
    return *cur_context;
  }

  void pkg::clear_hidden_function_symbol(symbols::symbol * sym) {
    if (sym) {
      sym->value = lisp_nil{};
      sym->value_info = value_info::value_info{imm_fn_ref, value_info::function_value_flags};
      sym->flags = symbols::symbol_flag::constant | symbols::symbol_flag::hidden;
      sym->state = symbols::symbol_state::uninitialized;
    }
  }

  J_RETURNS_NONNULL symbols::symbol * pkg::create_hidden_function_symbol(symbols::symbol ** J_NOT_NULL sym, strings::const_string_view name) {
    if (!*sym) {
      *sym = symbol_table.declare(
        emplace_id(name),
        lisp_nil{},
        symbols::symbol_flag::constant | symbols::symbol_flag::hidden,
        value_info::value_info{imm_fn_ref, value_info::function_value_flags});
    } else {
      clear_hidden_function_symbol(*sym);
    }
    return *sym;
  }

  J_RETURNS_NONNULL symbols::symbol * pkg::create_load_symbol() {
    return create_hidden_function_symbol(&load_symbol, ":load");
  }

  J_RETURNS_NONNULL symbols::symbol * pkg::create_eval_symbol() {
    return create_hidden_function_symbol(&eval_symbol, ":eval");
  }

  compilation::source_compilation_context & pkg::get_eval_compiler(sources::source * J_NOT_NULL src) {
    return compilation::get_source_compiler(*env->maybe_get_env_compiler(), *this, *src, create_eval_symbol());
  }

  symbols::symbol * pkg::compile_eval(sources::source * J_NOT_NULL src) {
    auto eval_symbol = compilation::compile_source(*env->maybe_get_env_compiler(), *this, *src, create_eval_symbol());
    {
      if (eval_symbol->value_info.is_function()) {
        auto & pkgc = get_pkg_context();
        compilation::context ctx(pkgc, *eval_symbol);
        compilation::compile_defun(ctx);
      }
    }
    return eval_symbol;
  }
  void pkg::clear_eval_symbol() {
    clear_hidden_function_symbol(eval_symbol);
  }

  lisp_imm pkg::eval(sources::source * J_NOT_NULL src) {
    auto eval_symbol = compile_eval(src);
    if (eval_symbol->value.is_fn_ref()) {
      lisp_imm result = lisp_imm{eval_symbol->value.as_fn_ref().value()(nullptr, 0U)};
      clear_hidden_function_symbol(eval_symbol);
      return result;
    } else {
      clear_hidden_function_symbol(eval_symbol);
      return lisp_nil{};
    }
  }
  id pkg::gensym() noexcept {
    return {gensym_v, symbol_table.names.package_id, gensym_counter++};
  }

  // lisp_imm pkg::eval(lisp_imm imm, sources::source * J_NOT_NULL src) {
  //   auto eval_symbol = compile_eval(src);
  // }

  functions::func_wrapper pkg::get_fn(id id) {
    symbols::symbol * sym = symbol_table.maybe_at(id);
    if (!sym || !sym->value_info.is_function()) {
      return {};
    }
    if (sym->is_uninitialized()) {
      compilation::compile_symbol(*this, *sym);
    }
    J_ASSERT(sym->is_initialized());
    J_ASSERT(sym->value.is_fn_ref());
    return {sym->value.as_fn_ref().value()};
  }

  functions::func_wrapper pkg::get_fn(const strings::const_string_view & name) {
    return get_fn(symbol_table.names.try_get_id_of(name));

  }
  functions::func_wrapper pkg::get_fn(const lisp_str * J_NOT_NULL name) {
    return get_fn(symbol_table.names.try_get_id_of(name->value()));
  }
}
