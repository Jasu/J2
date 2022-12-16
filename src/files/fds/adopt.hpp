#pragma once

namespace j::files::inline fds {
  struct fd;

  /// Create an FD object from an already-open file descriptor.
  ///
  /// Used to e.g. open the stdin/stdout/stderr streams.
  fd adopt(int fd_int);
}
