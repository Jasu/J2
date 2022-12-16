#pragma once

#include <termios.h>
#include "hzd/utility.hpp"

namespace j::tty {
  struct tty_restore_data final {
    int fd = -1;
    u32_t restore_sequence_length = 0U;
    bool is_dirty = false;
    bool has_termios = false;
    char restore_sequence[256];
    ::termios termios;
  };
}
