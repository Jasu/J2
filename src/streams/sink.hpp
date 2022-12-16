#pragma once

#include "strings/string_view.hpp"

namespace j::streams {
  /// Abstract base class for stream sinks
  class sink {
  public:
    /// Write at most num_bytes to the sink, returning the number of bytes actually written.
    ///
    /// \returns Number of bytes actually written to the sink.
    ///
    /// \notes Whether this blocks depends on the sink, and is not specified in the interface.
    virtual u32_t write(const char * from, u32_t num_bytes) noexcept = 0;

    u32_t write(const char * J_NOT_NULL str) noexcept;

    u32_t write(const strings::const_string_view & str) noexcept;

    /// Check whether the underlying file descriptor is a terminal.
    ///
    /// The default implementation always returns false.
    virtual bool is_tty() const;

    virtual ~sink();
  };
}
