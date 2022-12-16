#include "files/fds/file_type.hpp"

#include <sys/stat.h>

namespace j::files::inline fds {
  file_type mode_to_file_type(u32_t mode) noexcept {
    switch (mode & S_IFMT) {
    case S_IFBLK: return file_type::block_dev;
    case S_IFCHR: return file_type::char_dev;
    case S_IFDIR: return file_type::dir;
    case S_IFIFO: return file_type::fifo;
    case S_IFLNK: return file_type::symlink;
    case S_IFREG: return file_type::file;
    case S_IFSOCK: return file_type::socket;
    default:
      return file_type::none;
    }
  }
}
