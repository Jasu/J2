#pragma once

#include "util/flags.hpp"
#include "lisp/values/lisp_imm.hpp"
#include "lisp/value_info/value_info.hpp"
#include "lisp/common/id.hpp"

namespace j::lisp::compilation {
  struct tlf;
}
namespace j::lisp::symbols {
  enum class symbol_state : u8_t {
    pending,
    uninitialized,
    initializing,
    initialized,
  };

  enum class symbol_flag : u8_t {
    exported,
    constant,
    initialized,
    /// The symbol cannot be explicitly imported.
    ///
    /// E.g. the package's `:load` function is hidden, as are the
    /// `:function-name:closure-0` symbols. The hidden symbol names also begin
    /// with a colon, so they cannot even be lexed as normal, non-keyword symbols.
    hidden,
    requires_load_time_init,
  };

  using symbol_flags = util::flags<symbol_flag, u8_t>;
  J_FLAG_OPERATORS(symbol_flag, u8_t)

  struct symbol final {
    lisp_imm value;
    symbol_state state;
    symbol_flags flags;
    id name = id::none;

    value_info::value_info value_info;

    void notify_pending() noexcept;
    void set_pending(compilation::tlf * J_NOT_NULL pender) noexcept;

    J_INLINE_GETTER bool is_pending() const noexcept {
      return state == symbol_state::pending;
    }

    J_INLINE_GETTER bool is_exported() const noexcept {
      return flags.has(symbol_flag::exported);
    }

    J_INLINE_GETTER bool is_constant() const noexcept {
      return flags.has(symbol_flag::constant);
    }

    J_INLINE_GETTER bool is_writable() const noexcept {
      return !flags.has(symbol_flag::constant);
    }

    J_INLINE_GETTER bool is_hidden() const noexcept {
      return flags.has(symbol_flag::hidden);
    }

    J_INLINE_GETTER bool is_uninitialized() const noexcept {
      return state == symbol_state::uninitialized || state == symbol_state::pending;
    }

    J_INLINE_GETTER bool is_initializing() const noexcept {
      return state == symbol_state::initializing;
    }

    J_INLINE_GETTER bool is_initialized() const noexcept {
      return state == symbol_state::initialized;
    }

    J_INLINE_GETTER bool requires_load_time_init() const noexcept {
      return flags.has(symbol_flag::requires_load_time_init);
    }

    J_ALWAYS_INLINE void set_initializing() noexcept {
      state = symbol_state::initializing;
    }

    J_ALWAYS_INLINE void set_initialized() noexcept {
      J_ASSERT(state != symbol_state::initialized);
      state = symbol_state::initialized;
    }
  };
}
