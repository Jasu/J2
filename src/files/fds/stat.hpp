#pragma once

#include "files/fds/file_type.hpp"
#include "files/paths/path.hpp"
#include "hzd/utility.hpp"

namespace j::files::inline fds {
  struct fd;

  /// Information collected from k
  struct stat_info final {
    ssz_t size_bytes = -1;
    file_type type = file_type::none;
  };

  stat_info stat(int fd);
  stat_info stat(const fd & fd);

  stat_info stat_at(int fd, const path & path);
  stat_info stat_at(const fd & fd, const path & path);

  bool exists_at(int fd, const path & path);
  bool exists_at(const fd & fd, const path & path);

  bool dir_exists_at(int fd, const path & path);
  bool dir_exists_at(const fd & fd, const path & path);

  bool file_exists_at(int fd, const path & path);
  bool file_exists_at(const fd & fd, const path & path);

  J_INLINE_GETTER ssz_t file_size(const int & fd) {
    return stat(fd).size_bytes;
  }

  J_INLINE_GETTER ssz_t file_size(const fd & fd) {
    return stat(fd).size_bytes;
  }
}
