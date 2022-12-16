#include "process_info.hpp"

#include "strings/format.hpp"
#include "files/paths/path.hpp"
#include "files/fs.hpp"

#include <unistd.h>

namespace j::processes {
  static files::path get_proc_dir(pid_t pid) {
    J_ASSUME(pid > 0);
    return strings::format("/proc/{}/", (u64_t)pid);
  }

  files::path get_process_executable(pid_t pid) {
    files::path p = get_proc_dir(pid) / "exe";
    return files::fs::readlink(p);
  }

  pid_t get_self() {
    return getpid();
  }
}
