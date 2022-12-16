#pragma once

#include "hzd/types.hpp"

namespace j::files::inline fds {
  /// Type of a file / directory / whatever a file descriptor or an inode can point to.
  enum class file_type : u8_t {
    /// Unknown file descriptor type
    none      = 0U,

    /// Regular file `S_IFREG`
    file,
    /// Named pipe (FIFO) `S_IFIFO`
    fifo,
    /// Symbolic link `S_IFLNK`
    symlink,
    /// Directory `S_IFDIR`
    dir,
    /// Block debice `S_IFBLK`
    block_dev,
    /// Character device `S_IFCHR`
    char_dev,
    /// Socket `S_IFSOCK`
    socket,
  };

  /// Get the type of the file given an `st_mode` from stat.
  [[nodiscard]] file_type mode_to_file_type(u32_t mode) noexcept;
}
