#include "string_sink.hpp"

namespace j::streams {
  u32_t string_sink::write(const char * from, u32_t num_bytes) noexcept {
    if (num_bytes) {
      string.append(from, num_bytes);
    }
    return num_bytes;
  }
}
