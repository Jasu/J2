#pragma once

#include "lisp/values/imm_type.hpp"
#include "lisp/value_info/primitive_info.hpp"

namespace j::lisp::functions {
  struct func_info;
}

namespace j::lisp::value_info {

  enum class value_info_flag : u8_t {
    /// The value cannot change.
    ///
    /// \note For `vec`s, this applies to its immediate contents, not the values pointed to by them.
    immutable,
    /// Regular function.
    function,
    /// Macro function. Note that `function` is not set for macros.
    macro,
    /// Lambda function.
    lambda,
    /// Function is not a Lisp function. Note that `foreign` must be set.
    foreign,
  };


  using value_info_flags = util::flags<value_info_flag, u8_t>;
  J_FLAG_OPERATORS(value_info_flag, u8_t)

  constexpr inline value_info_flags function_value_flags{value_info_flag::function, value_info_flag::immutable};
  constexpr inline value_info_flags macro_value_flags{value_info_flag::macro, value_info_flag::immutable};
  constexpr inline value_info_flags lambda_value_flags{value_info_flag::lambda, value_info_flag::immutable};
  constexpr inline value_info_flags foreign_value_flags{value_info_flag::function, value_info_flag::foreign, value_info_flag::immutable};

  struct value_info final {
    J_BOILERPLATE(value_info, CTOR_CE)

    imm_type_mask types = any_imm_type;
    value_info_flags flags;
    bool_info bool_info{};
    i64_info i64_info{};
    functions::func_info * func_info = nullptr;

    J_ALWAYS_INLINE explicit value_info(imm_type_mask types, value_info_flags flags = {}) noexcept
      : types(types),
        flags(flags)
    {
    }

    J_INLINE_GETTER bool is_immutable() const noexcept {
      return flags.has(value_info_flag::immutable);
    }

    J_INLINE_GETTER bool is_function() const noexcept {
      return flags.has(value_info_flag::function);
    }

    J_INLINE_GETTER bool is_macro() const noexcept {
      return flags.has(value_info_flag::macro);
    }

    J_INLINE_GETTER bool is_foreign() const noexcept {
      return flags.has(value_info_flag::foreign);
    }
  };
}
