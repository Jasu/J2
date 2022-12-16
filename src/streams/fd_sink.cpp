#include "streams/fd_sink.hpp"

#include "streams/attributes.hpp"
#include "exceptions/assert_lite.hpp"
#include "files/fds/fd.hpp"
#include "mem/shared_ptr.hpp"
#include "services/service_definition.hpp"

#include <unistd.h>

namespace j::streams {
  fd_sink::fd_sink(const files::fd & fd)
    : m_fd(fd)
  {
    J_ASSERT_NOT_NULL(m_fd);
    J_REQUIRE(m_fd.can_write(), "Cannot create a sink from a non-writable FD.");
  }

  u32_t fd_sink::write(const char * from, u32_t num_bytes) noexcept {
    if (J_UNLIKELY(!num_bytes)) {
      return 0;
    }
    J_ASSERT_NOT_NULL(from, m_fd);
    auto result = ::write((int)m_fd, from, num_bytes);
    J_REQUIRE(result >= 0, "TODO Fix this require, handle signals etc.");
    return result;
  }

  void fd_sink::close() noexcept {
    m_fd.close();
  }

  bool fd_sink::is_tty() const {
    if (!m_has_is_tty_cached && m_fd) {
      m_has_is_tty_cached = true;
      m_is_tty_cached = m_fd.is_tty();
    }
    return m_is_tty_cached;
  }

  bool fd_sink::is_open() const noexcept {
    return (bool)m_fd;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<fd_sink> fd_sink_def(
      "streams.fd_sink",
      "FD Sink",
      s::implements = s::interface<sink>,
      s::create = s::constructor<files::fds::fd>(
        s::arg::service_attribute(attributes::fd)),
      s::no_default_instance
    );
  }
}
