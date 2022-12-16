#pragma once

#include "strings/string.hpp"
#include "streams/sink.hpp"

namespace j::streams {
  /// Sink for writing to file descriptors.
  struct string_sink : public sink {
    J_BOILERPLATE(string_sink, CTOR_CE, COPY_DEL)

    u32_t write(const char * from, u32_t num_bytes) noexcept override final;

    strings::string string;
  };
}
