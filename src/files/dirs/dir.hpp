#pragma once

#include "files/open_flags.hpp"
#include "files/fds/fd.hpp"
#include "files/paths/path.hpp"
#include "strings/string_view.hpp"
#include "files/fds/stat.hpp"

namespace j::files::dirs {
  struct dir final {
    J_BOILERPLATE(dir, CTOR_CE)

    /// Initialize `dir` without yet opening directory.
    explicit dir(paths::path && path) noexcept
      : path(static_cast<paths::path &&>(path))
    { }

    /// Initialize `dir` without yet opening directory.
    explicit dir(const paths::path & path) noexcept
      : path(path)
    { }

    /// Initialize `dir` to an already-opened directory.
    template<typename Fd, typename Path>
    dir(Fd && fd, Path && path) noexcept
      : path(static_cast<Path &&>(path)),
        m_fd(static_cast<Fd &&>(fd))
    { }

    fds::fd open_at(const paths::path & path, open_flags_t flags);

    fds::stat_info stat_at(const paths::path & path);

    bool exists_at(const paths::path & path);

    bool file_exists_at(const paths::path & path);

    bool dir_exists_at(const paths::path & path);

    void open();

    /// Get the file descriptor of the directory, opening the dir if not open.
    J_INLINE_GETTER fds::fd fd() {
      if (!m_fd) {
        open();
      }
      return m_fd;
    }

    paths::path path;

  private:
    fds::fd m_fd;
  };
}
