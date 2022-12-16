#pragma once

#include "util/flags.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::functions {
  enum class calling_convention : u8_t {
    none,
    /// Variables are passed as `lisp_imm` in registers or stack, as specified by host ABI.
    abi_call,
    /// Variables are passed as a pointer and count. Pointer in RDI, count in ESI.
    full_call,
    max_cc = full_call,
  };
  constexpr inline u8_t num_ccs = (u8_t)calling_convention::max_cc + 1U;
  constexpr inline calling_convention ccs[]{calling_convention::abi_call, calling_convention::full_call};

  constexpr inline strings::const_string_view cc_titles[num_ccs]{
    [(u8_t)calling_convention::none]      = "NoCC",
    [(u8_t)calling_convention::abi_call]  = "AbiCall",
    [(u8_t)calling_convention::full_call] = "FullCall",
  };

  constexpr inline strings::const_string_view cc_suffixes[num_ccs]{
    [(u8_t)calling_convention::none]      = "none",
    [(u8_t)calling_convention::abi_call]  = "ac",
    [(u8_t)calling_convention::full_call] = "fc",
  };

  using calling_conventions_t = util::flags<calling_convention, u8_t>;
  J_FLAG_OPERATORS(calling_convention, u8_t)

  constexpr inline calling_conventions_t all_calling_conventions{calling_convention::abi_call, calling_convention::full_call};

  template<typename T>
  struct cc_ptr_map {
    T * data[num_ccs] = { nullptr };

    J_INLINE_GETTER T * get(calling_convention cc) noexcept {
      return data[(u8_t)cc - 1];
    }

    J_INLINE_GETTER const T * get(calling_convention cc) const noexcept {
      return data[(u8_t)cc - 1];
    }

    J_INLINE_GETTER T * & operator[](calling_convention cc) noexcept {
      return data[(u8_t)cc - 1];
    }

    J_INLINE_GETTER const T * operator[](calling_convention cc) const noexcept {
      return data[(u8_t)cc - 1];
    }
  };

  template<typename T>
  struct cc_map {
    T data[num_ccs];

    J_INLINE_GETTER T & get(calling_convention cc) noexcept {
      return data[(u8_t)cc - 1];
    }

    J_INLINE_GETTER const T & get(calling_convention cc) const noexcept {
      return data[(u8_t)cc - 1];
    }

    J_INLINE_GETTER T & operator[](calling_convention cc) noexcept {
      return data[(u8_t)cc - 1];
    }

    J_INLINE_GETTER const T &  operator[](calling_convention cc) const noexcept {
      return data[(u8_t)cc - 1];
    }
  };
}
