#pragma once

#include "util/flags.hpp"

namespace j::files {
  enum class open_flags : u8_t {
    truncate,
    create,
    read,
    write,
    dir,
  };

  using open_flags_t = util::flags<open_flags, u8_t>;
  J_FLAG_OPERATORS(open_flags, u8_t)
}
