#pragma once

#include "files/paths/path.hpp"

namespace detail {
  class temp_file {
    j::files::path m_path;
  public:
    temp_file(const char * J_NOT_NULL contents);
    ~temp_file();

    J_INLINE_GETTER const j::files::path & path() const noexcept {
      return m_path;
    }
  };
}
