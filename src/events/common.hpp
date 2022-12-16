#pragma once

#include "util/flags.hpp"

extern "C" {
  typedef struct uv_loop_s uv_loop_t;
  typedef struct uv_handle_s uv_handle_t;
}

namespace j::events {
  struct event_loop;

  struct base_handle;
  struct stream_handle;
  struct signal_handle;
  struct poll_handle;
  struct prepare_handle;

  enum class handle_type : u8_t {
    none = 0,

    poll,
    tty,
    signal,
    prepare,
  };

  enum class poll_flag : u8_t {
    readable,
    writable,
  };

  using poll_flags = util::flags<poll_flag, u8_t>;
  J_FLAG_OPERATORS(poll_flag, u8_t)

  enum class read_flag : u8_t {
    read,
    eof,
  };

  using read_flags = util::flags<read_flag, u8_t>;
  J_FLAG_OPERATORS(read_flag, u8_t)

  enum class tty_mode : u8_t {
    none,

    normal,
    raw,
  };

  enum class eof_mode : u8_t {
    close,
    stop_reading,
  };
}
