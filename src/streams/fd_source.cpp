#include "streams/fd_source.hpp"

#include "mem/shared_ptr.hpp"
#include "exceptions/assert.hpp"
#include "files/fds/fd.hpp"
#include "services/service_definition.hpp"
#include "streams/attributes.hpp"

#include <unistd.h>

namespace j::streams {
  fd_source::fd_source(const files::fd & fd)
    : m_fd(fd)
  {
    J_ASSERT_NOT_NULL(m_fd);
    J_REQUIRE(m_fd.can_read(), "Cannot create a source from a closed or unreadable FD.");
  }

  u32_t fd_source::read(char * to, u32_t num_bytes) {
    if (J_UNLIKELY(!num_bytes)) {
      return 0;
    }
    J_ASSERT_NOT_NULL(to);
    auto result = ::write((int)m_fd, to, num_bytes);
    J_REQUIRE(result >= 0, "TODO Fix this require, handle signals etc.");
    return result;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<fd_source> fd_source_def(
      "streams.fd_source",
      "FD Source",
      s::implements = s::interface<source>,
      s::create = s::constructor<files::fds::fd>(
        s::arg::service_attribute(attributes::fd)),
      s::no_default_instance
    );
  }
}
