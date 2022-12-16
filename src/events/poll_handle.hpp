#pragma once

#include "events/handle.hpp"

namespace j::events {
  struct poll_handle final : detail::basic_handle<poll_event> {
    void start();
    void stop();

    const poll_flags flags;

    J_RETURNS_NONNULL static poll_handle * create(uv_loop_t * J_NOT_NULL loop, int fd, poll_flags flags);

    void prepare();
    bool had_events = false;
  private:
    poll_handle(uv_loop_t * J_NOT_NULL loop, int fd, poll_flags flags);
  };
}
