#include "services/metadata.hpp"

namespace j::services {
  class_metadata::class_metadata(const char * J_NOT_NULL name, const char * J_NOT_NULL title) noexcept
    : name(name),
      title(title)
  {
  }

  class_metadata::~class_metadata() {
  }
}
