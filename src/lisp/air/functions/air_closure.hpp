#pragma once

#include "lisp/air/analysis/lexical_scope.hpp"
#include "lisp/air/functions/closure_binding.hpp"
#include "lisp/compilation/const_table.hpp"

namespace j::lisp::functions {
  struct func_info;
}
namespace j::lisp::symbols {
  struct symbol;
}

namespace j::lisp::air::inline functions {
  struct activation_record;

  /// A single scope level (e.g. a `let`) wrapping a closure.
  struct closure_scope final {
    using iterator = closure_binding_iterator;
    using const_iterator = const_closure_binding_iterator;

    exprs::expr * expr = nullptr;
    closure_binding * first_binding = nullptr;
    u8_t loop_depth = 0U;
    u8_t closure_depth = 0U;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return first_binding;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !first_binding;
    }
    J_INLINE_GETTER bool empty() const noexcept {
      return !first_binding;
    }

    [[nodiscard]] bool contains(air_var * J_NOT_NULL var) {
      for (auto & v : *this) {
        if (&v == var) {
          return true;
        }
      }
      return false;
    }

    J_INLINE_GETTER iterator begin() noexcept {
      return {first_binding};
    }
    J_INLINE_GETTER iterator end() noexcept {
      return {nullptr};
    }

    J_INLINE_GETTER const_iterator begin() const noexcept {
      return {first_binding};
    }
    J_INLINE_GETTER const_iterator end() const noexcept {
      return {nullptr};
    }
  };

  struct air_closure final {
    J_ALWAYS_INLINE air_closure(u32_t scope_depth,
                                lisp::functions::func_info * J_NOT_NULL info,
                                symbols::symbol * sym,
                                air_closure * parent_closure) noexcept
      : parent_closure(parent_closure),
        func_info(info),
        closure_symbol(sym),
        scope_depth(scope_depth)
    {
    }

    air_closure * parent_closure = nullptr;
    /// Next closure in a linked list, stored in `air_function`.
    air_closure * next_closure = nullptr;

    activation_record * activation_record = nullptr;
    exprs::expr * expr = nullptr;
    lisp::functions::func_info * func_info = nullptr;
    symbols::symbol * closure_symbol = nullptr;

    /// Number of scopes wrapping this closure, possibly unused, possibly in different closures.
    u32_t scope_depth:8 = 0U;
    /// Highest scope number where a binding was made by this closure.
    u32_t max_binding_depth:8 = 0U;
    /// Index of the closure in the activation record.
    u32_t act_rec_closure_index:8 = 0U;

    /// Number of variables bound from upper scopes.
    ///
    /// Might be zero, plain functions returned by other functions will still be
    /// represented as closures, just with `nullptr` as their static chain. This
    /// makes it possible to treat any callable returned by a function the same.
    u32_t num_bindings = 0U;

    compilation::const_table constants{};


    span<closure_scope> scopes() noexcept {
      return { reinterpret_cast<closure_scope*>(this + 1), scope_depth };
    }

    span<const closure_scope> scopes() const noexcept {
      return { reinterpret_cast<const closure_scope*>(this + 1), scope_depth };
    }
  };
}
