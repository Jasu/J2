#pragma once

#include "exceptions/assert_lite.hpp"
#include "files/fds/file_type.hpp"
#include "files/fds/fd.hpp"

namespace j::files::inline fds {
  /// A file descriptor.
  struct fd_info final {
    J_BOILERPLATE(fd_info, CTOR_CE)

    constexpr fd_info(int fd, fd_flags flags, file_type type = file_type::none) noexcept
      : num(fd),
        flags(flags),
        type(type)
    { }

    void unref() noexcept {
      J_ASSUME(refs);
      if (!--refs && num) {
        close();
      }
    }

    void ref() noexcept {
      J_ASSUME(num >= 0);
      ++refs;
    }

    void close() noexcept;

    /// Get the underlying file descriptor as integer.
    J_INLINE_GETTER operator int() const noexcept { return num; }

    /// Return whether the file descriptor allows reading.
    J_INLINE_GETTER bool can_read() const noexcept {
      return num >= 0 && ((u8_t)flags & static_cast<u8_t>(fd_flags::read));
    }

    /// Return whether the file descriptor allows writing.
    J_INLINE_GETTER bool can_write() const noexcept {
      return num >= 0 && ((u8_t)flags & static_cast<u8_t>(fd_flags::write));
    }



    /// Return whether the FD points to a TTY.
    ///
    /// \note The result is cached after the first call.
    bool is_tty() const;

    /// Return whether the FD refers to a block device.
    J_INLINE_GETTER bool is_block_dev() const noexcept
    { return num >= 0 && type == file_type::block_dev; }

    /// Return whether the FD refers to a character device.
    J_INLINE_GETTER bool is_char_dev() const noexcept
    { return num >= 0 && type == file_type::char_dev; }

    /// Return whether the FD refers to a socket.
    J_INLINE_GETTER bool is_socket() const noexcept
    { return num >= 0 && type == file_type::socket; }

    /// Return whether the FD refers to a regular file.
    J_INLINE_GETTER bool is_file() const noexcept
    { return num >= 0 && type == file_type::file; }

    /// Return whether the FD refers to a FIFO/pipe.
    J_INLINE_GETTER bool is_fifo() const noexcept
    { return num >= 0 && type == file_type::fifo; }

    /// Return whether the FD refers to a directory.
    J_INLINE_GETTER bool is_dir() const noexcept
    { return num >= 0 && type == file_type::dir; }

    /// Return whether the FD refers to a symlink.
    ///
    /// \note Symlink FDs are obtained like directory FDs, with O_NOFOLLOW.
    J_INLINE_GETTER bool is_symlink() const noexcept
    { return num >= 0 && type == file_type::symlink; }

    J_INLINE_GETTER fd get() noexcept {
      ref();
      return fd(num);
    }

    /// Check whether the FD is invalid.
    J_INLINE_GETTER bool empty() const noexcept { return num < 0; }
    /// Check whether the FD is valid.
    J_INLINE_GETTER explicit operator bool() const noexcept { return num >= 0; }
    /// Check whether the FD is invalid.
    J_INLINE_GETTER bool operator!() const noexcept { return num < 0; }

    int num = -1;
    u32_t refs = 0;
    fd_flags flags = fd_flags::none;
    file_type type = file_type::none;
    mutable bool is_tty_value:1 = false;
    mutable bool is_tty_checked:1 = false;
  };

  struct fd_info_table final {
    J_RETURNS_NONNULL fd_info * at(int fd) noexcept;
    [[nodiscard]] fd add(int fd, fd_flags flags, file_type type = file_type::none) noexcept;
    [[nodiscard]] fd maybe_add(int fd, fd_flags flags, file_type type = file_type::none) noexcept;
  };

  extern constinit fd_info_table fd_table;
}
