#pragma once

#include <sys/types.h>

#include "files/paths/path.hpp"

namespace j::processes {
  files::path get_process_executable(pid_t pid);

  pid_t get_self();
}
