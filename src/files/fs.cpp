#include "files/fs.hpp"
#include "files/paths/path.hpp"
#include "exceptions/exceptions.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

namespace j::files::fs {
  bool exists(const path & path) {
    J_ASSERT_NOT_NULL(path);
    struct stat stat_buffer;
    errno = 0U;
    int result = ::stat(path.as_c_string(), &stat_buffer);
    return result == 0;
  }

  void mkdir(const path & path) {
    J_ASSERT_NOT_NULL(path);
    errno = 0U;
    int result = ::mkdir(path.as_c_string(), S_IRWXU);
    J_REQUIRE(!result, "Could not create directory {} ({}).", path, result);
  }

  path readlink(const path & path) {
    J_ASSERT_NOT_NULL(path);
    char path_buf[256];
    errno = 0U;
    ssz_t sz = ::readlink(path.as_c_string(), path_buf, sizeof(path_buf));
    if (J_UNLIKELY(sz < 0)) {
      J_THROW(system_error_exception() << message("readlink failed"));
    } else if (J_UNLIKELY(sz == 256)) {
      // Maybe truncated - retry.
      char * big_buf = ::new char[4096];
      errno = 0;
      sz = ::readlink(path.as_c_string(), big_buf, 4096);
      if (J_UNLIKELY(sz < 0)) {
        ::delete[] big_buf;
        J_THROW(system_error_exception() << message("readlink failed."));
      } else if (J_UNLIKELY(sz == 4096)) {
        ::delete[] big_buf;
        J_THROW(system_error_exception() << message("readlink path too long."));
      }
      big_buf[sz] = 0;
      class path p(strings::string(big_buf, sz + 1U));
      ::delete[] big_buf;
      return p;
    }

    path_buf[sz] = 0;
    return strings::string(path_buf, sz + 1U);
  }
}
