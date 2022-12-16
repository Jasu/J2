#pragma once

#include "streams/sink.hpp"

namespace j::streams {
  class buffered_sink : public sink {
  public:
    virtual char * get_write_buffer(u32_t num_bytes) = 0;
    virtual void flush() = 0;
  };
}
