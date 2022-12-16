#include "files/fds/adopt.hpp"
#include "files/fds/fd.hpp"
#include "exceptions/assert.hpp"
#include "files/fds/fd_info.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

namespace j::files::inline fds {
  fd adopt(int fd_int) {
    J_REQUIRE(fd_int >= 0, "Cannot adopt a negative file descriptor.");
    struct stat stat_buffer;
    int result = ::fstat(fd_int, &stat_buffer);
    J_REQUIRE(!result, "fstat failed.");

    file_type t = mode_to_file_type(stat_buffer.st_mode);

    fd_flags f;
    int flags = ::fcntl(fd_int, F_GETFL);
    switch (flags & O_ACCMODE) {
    case O_RDONLY:
      f = fd_flags::read;
      break;
    case O_WRONLY:
      f = fd_flags::write;
      break;
    case O_RDWR:
      f = fd_flags::read_write;
      break;
    default:
      J_THROW("Unknown O_ACCMODE.");
    }

    return fd_table.maybe_add(fd_int, f, t);
  }
}
