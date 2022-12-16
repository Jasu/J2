#pragma once

namespace j::tty {
  struct tty_restore_data;

  tty_restore_data * get_tty_restore_data() noexcept;
  void mark_tty_dirty() noexcept;
  void mark_tty_clean() noexcept;
  void restore_tty_mode() noexcept;
}
