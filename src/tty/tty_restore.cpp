#include "tty/tty_restore.hpp"
#include "tty/tty_restore_data.hpp"

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

namespace j::tty {
  namespace {
    tty_restore_data tty_data;
    bool has_handlers = false;

    void do_restore_tty_mode(bool is_signal) noexcept {
      if (!tty_data.is_dirty || tty_data.fd < 0) {
        return;
      }

      if (tty_data.restore_sequence_length) {
        write(tty_data.fd, tty_data.restore_sequence, tty_data.restore_sequence_length);
        tty_data.restore_sequence_length = 0U;
      }

      if (tty_data.has_termios) {
        tcsetattr(tty_data.fd, is_signal ? TCSANOW : TCSAFLUSH, &tty_data.termios);
      }
      tty_data.is_dirty = false;
    }

    void restore_tty_mode_signal(int signum) {
      do_restore_tty_mode(true);
      _exit(128 + signum);
    }
  }

  tty_restore_data * get_tty_restore_data() noexcept {
    return &tty_data;
  }

  void mark_tty_dirty() noexcept {
    tty_data.is_dirty = true;
    if (!has_handlers) {
      has_handlers = true;
      atexit(restore_tty_mode);

      struct sigaction sa;
      sa.sa_handler = &restore_tty_mode_signal;
      sigemptyset(&sa.sa_mask);
      // sigaddset(&sa.sa_mask, SIGINT);
      sa.sa_flags = 0;
      sigaction(SIGINT, &sa, nullptr);
    }
  }

  void mark_tty_clean() noexcept {
    tty_data.is_dirty = false;
  }

  void restore_tty_mode() noexcept {
    do_restore_tty_mode(false);
  }
}
