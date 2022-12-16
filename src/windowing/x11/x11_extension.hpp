#pragma once

#include "hzd/utility.hpp"

extern "C" {
  struct xcb_extension_t;
}

namespace j::windowing::x11 {
  class context;

  struct x11_extension final {
    J_ALWAYS_INLINE constexpr x11_extension() noexcept = default;

    x11_extension(context & ctx, xcb_extension_t * J_NOT_NULL ext);

    J_INLINE_GETTER bool exists() const noexcept
    { return opcode; }

    u8_t opcode = 0U;
    u8_t first_event = 0U;
    u8_t first_error = 0U;
  };
}
