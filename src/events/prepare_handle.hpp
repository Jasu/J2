#pragma once

#include "events/handle.hpp"

namespace j::events {
  struct prepare_handle final : detail::basic_handle<prepare_event> {
    void start();
    void stop();
    J_RETURNS_NONNULL static prepare_handle * create(uv_loop_t * J_NOT_NULL loop);

    void prepare();
  private:
    prepare_handle(uv_loop_t * J_NOT_NULL loop);
  };
}
