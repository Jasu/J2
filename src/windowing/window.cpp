#include "windowing/window.hpp"

namespace j::windowing {
  window::window(u16_t initial_width, u16_t initial_height) noexcept
    : width(initial_width),
      height(initial_height)
  {
  }
  window::~window() {
  }

  void window::request_close() {
    if (on_close_request(signals::aggregate_all)) {
      destroy();
    }
  }
}
