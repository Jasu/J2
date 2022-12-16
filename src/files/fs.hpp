#pragma once

namespace j::files {
  inline namespace paths {
    class path;
  }

  namespace fs {
    bool exists(const path & path);
    void mkdir(const path & path);
    path readlink(const path & path);
  }
}
