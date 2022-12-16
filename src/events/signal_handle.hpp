#pragma once

#include "events/handle.hpp"

namespace j::events {
  struct request;

  struct signal_handle final : detail::basic_handle<signal_event> {
    void start();
    void stop();
    J_RETURNS_NONNULL static signal_handle * create(uv_loop_t * J_NOT_NULL loop, int signum);

    void prepare();
  private:
    signal_handle(uv_loop_t * J_NOT_NULL loop, int signum);
  };
}
