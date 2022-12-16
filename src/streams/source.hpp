#pragma once

#include "hzd/types.hpp"

namespace j::streams {
  /// Abstract base class for stream sinks
  class source {
  public:
    /// Read at most num_bytes from the source, returning the number of bytes actually read.
    ///
    /// \returns Number of bytes actually read from the source.
    ///
    /// \note Whether this blocks depends on the source, and is not specified in the interface.
    virtual u32_t read(char * to, u32_t num_bytes) = 0;

    virtual ~source();
  };

}
