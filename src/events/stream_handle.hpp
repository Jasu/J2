#pragma once

#include "events/handle.hpp"
#include "geometry/vec2.hpp"

namespace j::events {
  struct request;

  struct stream_handle final : detail::basic_handle<read_event, write_event> {
    J_INLINE_GETTER bool is_tty() const noexcept {
      return type == handle_type::tty;
    }

    void start();
    void stop();

    void write(mem::const_memory_region buf, write_handler on_write = {}, error_handler on_error = {});
    void write(strings::const_string_view buf, write_handler on_write = {}, error_handler on_error = {});
    void flush();

    geometry::vec2i32 tty_size() noexcept;

    void on_finish(request * J_NOT_NULL req);

    J_RETURNS_NONNULL static stream_handle * create(uv_loop_t * J_NOT_NULL loop, int fd, eof_mode eof_handling);

    ~stream_handle();
    void prepare();

    eof_mode eof_handling;
  private:
    stream_handle(uv_loop_t * J_NOT_NULL loop, int fd, eof_mode eof_handling);
  };
}
