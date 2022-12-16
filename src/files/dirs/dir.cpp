#include "files/dirs/dir.hpp"
#include "files/fds/open.hpp"

namespace j::files::dirs {
  void dir::open() {
    J_ASSERT(!m_fd);
    J_ASSERT_NOT_NULL(path);
    m_fd = fds::open(path, open_flags::read | open_flags::dir);
    J_ASSERT(m_fd.is_dir());
  }

  fds::fd dir::open_at(const paths::path & path, open_flags_t flags) {
    J_ASSERT_NOT_NULL(path);
    return fds::open_at(m_fd, path, flags);
  }

  fds::stat_info dir::stat_at(const paths::path & path) {
    J_ASSERT_NOT_NULL(path);
    return fds::stat_at(fd(), path);
  }

  bool dir::exists_at(const paths::path & path) {
    J_ASSERT_NOT_NULL(path);
    return fds::exists_at(fd(), path);
  }

  bool dir::dir_exists_at(const paths::path & path) {
    J_ASSERT_NOT_NULL(path);
    return fds::dir_exists_at(fd(), path);
  }

  bool dir::file_exists_at(const paths::path & path) {
    J_ASSERT_NOT_NULL(path);
    return fds::file_exists_at(fd(), path);
  }
}
