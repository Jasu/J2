#include "files/fds/open.hpp"
#include "files/paths/path.hpp"
#include "exceptions/assert.hpp"
#include "exceptions/exceptions.hpp"
#include "files/fds/fd_info.hpp"
#include "files/fds/stat.hpp"
#include "containers/pair.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace j::files::inline fds {
  namespace {
    fd handle_fd(int result, fd_flags fdf, const path & path) {
      if (result < 0) {
        exceptions::throw_exception(exceptions::system_error_exception()
                                    << exceptions::message("Could not open file \"{}\".")
                                    << exceptions::value(path));
      }
      auto stat_info = stat(result);
      return fd_table.add(result, fdf, stat_info.type);
    }

    pair<int, fd_flags> parse_flags(open_flags_t flags) {
      int f = O_WRONLY;
      fd_flags fdf = fd_flags::none;
      if (flags.has(open_flags::dir)) {
        f |= O_DIRECTORY;
      }

      if (flags.has(open_flags::read)) {
        f = flags.has(open_flags::write) ? O_RDWR : O_RDONLY;
        fdf = flags.has(open_flags::write) ? fd_flags::read_write : fd_flags::read;
      } else {
        J_ASSERT(flags.has(open_flags::write), "Must specify some access mode.");
        fdf = fd_flags::write;
      }
      if (flags.has(open_flags::create)) {
        f |= O_CREAT;
      }
      if (flags.has(open_flags::truncate)) {
        f |= O_TRUNC;
      }
      return {f, fdf};
    }
  }

  fd open(strings::const_string_view path, open_flags_t flags) {
    J_REQUIRE_NOT_NULL(path);
    auto p = parse_flags(flags);
    mode_t m = S_IRUSR | S_IWUSR;
    if (!path.is_null_terminated()) {
      char * buf = (char*)J_ALLOCA(path.size() + 1);
      j::memcpy(buf, path.begin(), path.size());
      buf[path.size()] = 0;
      path.m_data = buf;
    }
    return handle_fd(::open(path.begin(), p.first, m), p.second, path);
  }

  fd open(const path & path, open_flags_t flags) {
    J_REQUIRE_NOT_NULL(path);
    auto p = parse_flags(flags);
    mode_t m = S_IRUSR | S_IWUSR;
    return handle_fd(::open(path.as_c_string(), p.first, m), p.second, path);
  }

  fd open_at(const fd & at, const path & path, open_flags_t flags) {
    J_REQUIRE_NOT_NULL(path, at);
    J_ASSERT(at.is_dir());
    auto p = parse_flags(flags);
    mode_t m = S_IRUSR | S_IWUSR;
    return handle_fd(::openat(at.num, path.as_c_string(), p.first, m), p.second, path);
  }
}
