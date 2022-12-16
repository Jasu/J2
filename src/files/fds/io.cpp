#include "files/fds/io.hpp"
#include "files/fds/fd.hpp"
#include "exceptions/exceptions.hpp"
#include "mem/buffer_builder.hpp"

#include <unistd.h>
#include <cerrno>

namespace j::files::inline fds {
  io_result read(const fd & file, void * J_NOT_NULL buf, u32_t bytes) {
    J_ASSERT(file.can_read());
  retry:
    errno = 0;
    ssz_t result = ::read(file.num, buf, bytes);
    if (J_UNLIKELY(result < 0)) {
      if (result == EINTR) {
        goto retry;
      }
      J_SYS_ERR("read from {} failed", file.num);
    }
    return io_result(result);
  }

  io_result write(const fd & file, const void * J_NOT_NULL buf, u32_t bytes) {
    J_ASSERT(file.can_write());
  retry:
    errno = 0;
    ssz_t result = ::write(file.num, buf, bytes);
    if (J_UNLIKELY(result < 0)) {
      if (result == EINTR) {
        goto retry;
      }
      J_SYS_ERR("write to {} failed", file.num);
    }
    return io_result(result);
  }

  mem::buffer read_to_end(const fd & file, u32_t num_zero_terminators) {
    mem::buffer_builder b;
    for (;;) {
      u32_t avail = b.size_available();
      if (avail < 64U) {
        b.reserve(avail + 1U);
        avail = b.size_available();
        J_ASSERT(avail > 0U);
      }

      auto res = b.reserve(avail);
      io_result iores = read(file, res, avail);

      if (!iores) {
        if (num_zero_terminators) {
          ::j::memzero(b.allocate(num_zero_terminators), num_zero_terminators);
        }
        return b.build();
      }

      J_ASSERT(iores.value() <= (i32_t)avail);
      res = add_bytes(res, iores.value());
      b.commit(res);
    }
  }
}
