#include "files/fds/adopt.hpp"
#include "files/fds/fd.hpp"
#include "streams/fd_sink.hpp"
#include "streams/fd_source.hpp"
#include "services/service_instance.hpp"
#include "streams/attributes.hpp"

#include <unistd.h>

namespace j::files::inline fds {
  namespace {
    fd get_stderr_fd() {
      return adopt(STDERR_FILENO);
    }

    fd get_stdout_fd() {
      return adopt(STDOUT_FILENO);
    }

    fd get_stdin_fd() {
      return adopt(STDIN_FILENO);
    }

    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<streams::fd_sink> stdout_def(
      "files.stdout",
      "stdout",
      streams::attributes::fd = get_stdout_fd());

    J_A(ND, NODESTROY) const s::service_instance<streams::fd_sink> stderr_def(
      "files.stderr",
      "stderr",
      streams::attributes::fd = get_stderr_fd());

    J_A(ND, NODESTROY) const s::service_instance<streams::fd_source> stdin_def(
      "files.stdin",
      "stdin",
      streams::attributes::fd = get_stdin_fd());
  }
}
