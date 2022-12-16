#pragma once

#include "streams/sink.hpp"
#include "files/fds/fd.hpp"

namespace j::files::inline fds {
  struct fd;
}

namespace j::streams {
  /// Sink for writing to file descriptors.
  class fd_sink : public sink {
  public:
    constexpr fd_sink() noexcept = default;

    explicit fd_sink(const files::fd &fd);

    u32_t write(const char * from, u32_t num_bytes) noexcept override final;

    bool is_tty() const override;

    void close() noexcept;

    bool is_open() const noexcept;

    fd_sink(fd_sink &&) noexcept = default;
    fd_sink & operator=(fd_sink &&) noexcept = default;
  private:
    files::fd m_fd;
    mutable bool m_has_is_tty_cached = false;
    mutable bool m_is_tty_cached = false;
  };
}
