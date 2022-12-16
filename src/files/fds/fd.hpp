#pragma once

#include "exceptions/assert_lite.hpp"
#include "files/fds/file_type.hpp"

namespace j::files::inline fds {
  /// Open flags of FD object.
  enum class fd_flags : u8_t {
    none             = 0U,
    read             = 1U,
    write            = 2U,
    read_write       = 3U
  };

  /// A file descriptor.
  struct fd final {
    J_BOILERPLATE(fd, CTOR_CE)

    J_A(AI,ND,HIDDEN) fd(fd && rhs) noexcept
      : num(rhs.num)
    { rhs.num = -1; }

    fd(const fd & rhs) noexcept;

    fd & operator=(const fd & rhs) noexcept;

    ~fd();

    fd & operator=(fd && rhs) noexcept;

    void close() noexcept;

    /// Get the underlying file descriptor as integer.
    J_A(AI,ND,HIDDEN,NODISC) inline explicit operator int() const noexcept { return num; }

    /// Return whether the file descriptor allows reading.
    [[nodiscard]] bool can_read() const noexcept;

    /// Return whether the file descriptor allows writing.
    [[nodiscard]] bool can_write() const noexcept;

    /// Return whether the FD points to a TTY.
    ///
    /// \note The result is cached after the first call.
    [[nodiscard]] bool is_tty() const;


    /// Return whether the FD refers to a block device.
    [[nodiscard]] bool is_block_dev() const noexcept;

    /// Return whether the FD refers to a character device.
    [[nodiscard]] bool is_char_dev() const noexcept;

    /// Return whether the FD refers to a socket.
    [[nodiscard]] bool is_socket() const noexcept;

    /// Return whether the FD refers to a regular file.
    [[nodiscard]] bool is_file() const noexcept;

    /// Return whether the FD refers to a FIFO/pipe.
    [[nodiscard]] bool is_fifo() const noexcept;

    /// Return whether the FD refers to a directory.
    [[nodiscard]] bool is_dir() const noexcept;

    /// Return whether the FD refers to a symlink.
    ///
    /// \note Symlink FDs are obtained like directory FDs, with O_NOFOLLOW.
    [[nodiscard]] bool is_symlink() const noexcept;


    /// Check whether the FD is valid.
    J_A(AI,ND,HIDDEN,NODISC) inline explicit operator bool() const noexcept { return num >= 0; }
    /// Check whether the FD is invalid.
    J_A(AI,ND,HIDDEN,NODISC) inline bool operator!() const noexcept { return num < 0; }

    [[nodiscard]] file_type type() const noexcept;

    int num = -1;
  private:
    J_A(AI,ND,HIDDEN) inline explicit fd(int fd) noexcept
      : num(fd)
    { }

    friend struct fd_info;
  };
}
