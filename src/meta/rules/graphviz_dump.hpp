#pragma once

#include "files/paths/path.hpp"

namespace j::meta::inline rules {
  struct grammar;

  void graphviz_dump(const files::path & path, grammar & g);
}
