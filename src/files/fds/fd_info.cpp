#include "files/fds/fd_info.hpp"
#include "containers/vector.hpp"
#include <unistd.h>
#include <cerrno>

namespace j::files::inline fds {
  namespace {
    constinit noncopyable_vector<j::files::fd_info> fd_vec;
  }

  void fd_info::close() noexcept {
    J_ASSERT(num >= 0, "Tried to close a non-open file descriptor.");
    ::close(num);
    num = -1;
    flags = fd_flags::none;
    is_tty_checked = false;
    is_tty_value = false;
  }

  bool fd_info::is_tty() const {
    J_REQUIRE(num >= 0, "Tried to check TTY status of a closed FD.");
    if (!is_tty_checked) {
      if (isatty(num)) {
        is_tty_value = true;
      } else if (errno == ENOTTY) {
        errno = 0;
      } else {
        J_SYS_ERR("isatty failed for {}", num);
      }
      is_tty_checked = true;
    }
    return is_tty_value;
  }
  J_RETURNS_NONNULL fd_info * fd_info_table::at(int fd) noexcept {
    J_ASSERT_RANGE(0, fd, (int)fd_vec.size());
    auto res = &fd_vec.at(fd);
    J_ASSERT(res->num == fd);
    return res;
  }
  fd fd_info_table::add(int fd, fd_flags flags, file_type type) noexcept {
    J_ASSUME(fd >= 0);
    if ((u32_t)fd >= fd_vec.size()) {
      fd_vec.resize(max(fd + 1, 32));
    }
    auto & res = fd_vec[fd];
    J_ASSUME(res.num == -1);
    J_ASSUME(!res.refs);
    res = {fd, flags, type};
    return res.get();
  }

  fd fd_info_table::maybe_add(int fd, fd_flags flags, file_type type) noexcept {
    J_ASSUME(fd >= 0);
    if ((u32_t)fd < fd_vec.size()) {
      if (auto & res = fd_vec[fd]) {
        J_ASSUME(res.num == fd);
        J_ASSUME(res.refs);
        return res.get();
      }
    }
    return add(fd, flags, type);
  }


  constinit fd_info_table fd_table;
}
