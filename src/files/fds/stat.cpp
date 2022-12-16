#include "files/fds/stat.hpp"
#include "files/fds/fd.hpp"
#include "exceptions/assert.hpp"
#include "files/paths/path.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace j::files::inline fds {
  namespace {
    stat_info handle_stat_result(int result, const struct ::stat & stat_buffer) {
      // TODO: Throw a better error.
      if (result) {
        J_SYS_ERR("stat failed");
      }
      return stat_info{
        stat_buffer.st_size,
        mode_to_file_type(stat_buffer.st_mode),
      };
    }
  }

  stat_info stat(int fd) {
    struct ::stat stat_buffer;
    return handle_stat_result(::fstat(fd, &stat_buffer), stat_buffer);
  }

  stat_info stat(const fd & fd) {
    return stat((int)fd);
  }

  stat_info stat_at(int fd, const path & path) {
    J_ASSERT(fd >= 0);
    J_ASSERT_NOT_NULL(path);
    struct ::stat stat_buffer;
    return handle_stat_result(::fstatat(fd, path.as_c_string(), &stat_buffer, 0), stat_buffer);
  }

  stat_info stat_at(const fd & fd, const path & path) {
    return stat_at((int)fd, path);
  }

  bool exists_at(int fd, const path & path) {
    J_ASSERT(fd >= 0);
    J_ASSERT_NOT_NULL(path);
    struct ::stat stat_buffer;
    return ::fstatat(fd, path.as_c_string(), &stat_buffer, 0) == 0;
  }

  bool exists_at(const fd & fd, const path & path) {
    return exists_at((int)fd, path);
  }

  bool dir_exists_at(int fd, const path & path) {
    J_ASSERT(fd >= 0);
    J_ASSERT_NOT_NULL(path);
    struct ::stat stat_buffer;
    return ::fstatat(fd, path.as_c_string(), &stat_buffer, 0) == 0 && S_ISDIR(stat_buffer.st_mode);
  }

  bool dir_exists_at(const fd & fd, const path & path) {
    return dir_exists_at((int)fd, path);
  }

  bool file_exists_at(int fd, const path & path) {
    J_ASSERT(fd >= 0);
    J_ASSERT_NOT_NULL(path);
    struct ::stat stat_buffer;
    return ::fstatat(fd, path.as_c_string(), &stat_buffer, 0) == 0 && S_ISREG(stat_buffer.st_mode);
  }

  bool file_exists_at(const fd & fd, const path & path) {
    return file_exists_at((int)fd, path);
  }
}
