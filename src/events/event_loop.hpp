#pragma once

#include "events/common.hpp"

namespace j::events {
  /// Interface for event loops.
  struct alignas(16) event_loop final {
    J_BOILERPLATE(event_loop, COPY_DEL, MOVE_DEL)

    event_loop();

    ~event_loop();

    J_RETURNS_NONNULL poll_handle * add_poll_handle(int fd, poll_flags flags);
    J_RETURNS_NONNULL stream_handle * add_tty_handle(int fd);
    J_RETURNS_NONNULL signal_handle * add_signal_handle(int signum);
    J_RETURNS_NONNULL prepare_handle * add_prepare_handle();

    [[nodiscard]] poll_handle * find_poll_handle(int fd);
    [[nodiscard]] stream_handle * find_stream_handle(int fd);
    [[nodiscard]] signal_handle * find_signal_handle(int signum);

    void remove_poll_handle(int fd);

    void remove_tty_handle(int fd);

    void run();

    void stop();

    void cycle();

    bool is_running = false;

    void on_before_delete(base_handle * J_NOT_NULL handle);

    J_RETURNS_NONNULL uv_loop_t * uv_loop() noexcept;
  };
}
