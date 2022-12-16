#include "files/fds/fd.hpp"
#include "files/fds/fd_info.hpp"
#include "exceptions/assert.hpp"
#include "services/interface_definition.hpp"

namespace j::files::inline fds {
  fd::fd(const fd & rhs) noexcept
    : num(rhs.num)
  {
    if (num >= 0) {
      fd_table.at(num)->ref();
    }
  }

  fd & fd::operator=(const fd & rhs) noexcept {
    if (num != rhs.num) {
      if (num >= 0) {
        fd_table.at(num)->unref();
      }
      num = rhs.num;
      if (num >= 0) {
        fd_table.at(num)->ref();
      }
    }
    return *this;
  }

  fd & fd::operator=(fd && rhs) noexcept {
    if (num != rhs.num) {
      if (num >= 0) {
        fd_table.at(num)->unref();
      }
      num = rhs.num;
      rhs.num = -1;
    }
    return *this;
  }

  fd::~fd() {
    if (num >= 0) {
      fd_table.at(num)->unref();
    }
  }

  void fd::close() noexcept {
    fd_table.at(num)->close();
    num = -1;
  }

  bool fd::is_tty() const {
    return fd_table.at(num)->is_tty();
  }

  bool fd::can_read() const noexcept {
    return fd_table.at(num)->can_read();
  }

  bool fd::can_write() const noexcept {
    return fd_table.at(num)->can_write();
  }

  bool fd::is_block_dev() const noexcept {
    return fd_table.at(num)->is_block_dev();
  }

  bool fd::is_char_dev() const noexcept {
    return fd_table.at(num)->is_char_dev();
  }

  bool fd::is_socket() const noexcept {
    return fd_table.at(num)->is_socket();
  }

  bool fd::is_file() const noexcept {
    return fd_table.at(num)->is_file();
  }

  bool fd::is_fifo() const noexcept {
    return fd_table.at(num)->is_fifo();
  }

  bool fd::is_dir() const noexcept {
    return fd_table.at(num)->is_dir();
  }

  bool fd::is_symlink() const noexcept {
    return fd_table.at(num)->is_symlink();
  }

  file_type fd::type() const noexcept {
    return fd_table.at(num)->type;
  }

  namespace {
    J_A(ND, NODESTROY) const services::interface_definition<fd> fd_def("files.fd", "File descriptor");
  }
}
