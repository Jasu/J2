#pragma once

#include "hzd/utility.hpp"

namespace j::streams {
  enum stream_status : u8_t {
    s_tty          = 0b00000001,
    s_is_open      = 0b00000010,
    s_tty_unknown  = 0b00010000,
    s_at_eof       = 0b00100000,
    s_closed       = 0b01000000,
    s_error        = 0b10000000,
  };


  struct stream;

  using stream_write_fn = i32_t (*) (stream * J_AA(NOALIAS,NN), const void * J_AA(NOALIAS,NN), i32_t) noexcept;
  using stream_read_fn = i32_t (*) (stream * J_AA(NOALIAS,NN), void * J_AA(NOALIAS,NN), i32_t) noexcept;
  // using stream_seek_fn = i32_t (*) (stream * J_AA(NN), i32_t, stream_at) noexcept;
  // using stream_tell_fn = i32_t (*) (stream * J_AA(NN)) noexcept;

  enum stream_service_param : u32_t {
    s_close     = 1 << 0,
    s_flush     = 1 << 1,
    s_check_tty = 1 << 2,
    s_free      = 1 << 3,
  };

  using stream_service_fn = int (*) (stream * J_AA(NN), stream_service_param) noexcept;

  struct stream final {
    stream_read_fn read_fn;
    stream_write_fn write_fn;
    // stream_seek_fn seek_fn;
    // stream_tell_fn tell_fn;
    stream_service_fn service_fn;

    u8_t status;

    J_A(AI,ND,HIDDEN,NODISC) inline i32_t read(void * J_AA(NOALIAS,NN) to, i32_t sz) noexcept {
      return read_fn(this, to, sz);
    }

    J_A(AI,ND,HIDDEN) inline i32_t write(const void * J_AA(NOALIAS,NN) to, i32_t sz) noexcept {
      return write_fn(this, to, sz);
    }

    J_A(AI,ND,HIDDEN) inline i32_t flush() noexcept {
      return service_fn(this, s_flush);
    }

    J_A(AI,ND,HIDDEN) inline i32_t close() noexcept {
      return service_fn(this, s_close);
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_ok() const noexcept {
      return status <= s_tty_unknown;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_ok_tty() const noexcept {
      return status == (s_tty | s_is_open);
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_at_eof() const noexcept {
      return status & s_at_eof;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_open() const noexcept {
      return status & s_is_open;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool has_error() const noexcept {
      return status >= s_error;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool is_ok_tty_check() noexcept {
      return status == (s_tty | s_is_open) ? true
        : status == s_is_open ? service_fn(this, s_check_tty) : false;
    }

    J_A(AI,ND,HIDDEN) static inline void release(stream * J_AA(NN) s) noexcept{
      s->service_fn(s, s_free);
    }
  };
}
