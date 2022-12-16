#include "lisp/compilation/context.hpp"

#include "lisp/packages/pkg.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/values/lisp_vec.hpp"
#include "strings/format.hpp"

namespace j::lisp::compilation {
  context::context(pkg_context & package_context, symbols::symbol & sym) noexcept
    : pkg(package_context.pkg),
      env_compiler(package_context.env_compiler),
      temp_pool(env_compiler.temp_pool),
      symbol(sym),
      package_context(package_context),
      temp_checkpoint(temp_pool.checkpoint())
  {
    func_info = nullptr;
    J_ASSERT(symbol.state == symbols::symbol_state::uninitialized || symbol.state == symbols::symbol_state::pending);
    symbol.state = symbols::symbol_state::initializing;
  }

  context::~context() {
    J_ASSERT(symbol.state == symbols::symbol_state::initializing);
    symbol.state = symbols::symbol_state::initialized;
    temp_pool.rewind_to_checkpoint(temp_checkpoint);
  }

  strings::const_string_view context::name() const noexcept {
    return pkg.symbol_table.names.name_of(symbol.name)->value();
  }

  [[nodiscard]] J_RETURNS_NONNULL symbols::symbol * context::alloc_closure_symbol(u32_t index, functions::func_info * info) {
    strings::string n = strings::format(":{}:closure-{}", name(), index);
    auto sym = pkg.symbol_table.declare(
      pkg.symbol_table.names.emplace(n, &env_compiler.env->heap),
      symbols::symbol_flag::constant | symbols::symbol_flag::hidden);
    sym->value_info = value_info::value_info(imm_fn_ref, value_info::function_value_flags);
    sym->state = symbols::symbol_state::initializing;
    sym->value_info.func_info = info;
    return sym;
  }

  lisp_vec_ref context::allocate_vec(u32_t sz) {
    return lisp_vec_ref(lisp_vec::allocate(env_compiler.env->heap, sz, (lisp::mem::object_hightag_flag)0, 1U));
  }

  lisp_vec_ref context::allocate_vec(u32_t sz, const sources::source_location & loc) {
    return lisp_vec_ref(lisp_vec::allocate_with_debug_info(env_compiler.env->heap, loc, sz, (lisp::mem::object_hightag_flag)0, 1U));
  }
}
