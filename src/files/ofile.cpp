#include "files/ofile.hpp"
#include "files/fds/open.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::files {
  ofile::ofile(const path & p, open_flags_t f)
    : streams::fd_sink(fds::open(p, open_flags::write | f))
  {
  }

  void ofile::open(const path & p, open_flags_t f) {
    J_REQUIRE(!is_open(), "File is already open.");

    (streams::fd_sink &)*this = streams::fd_sink(fds::open(p, open_flags::write | f));
  }

  void ofile::write_bytes(const char * from, u32_t num_bytes) {
    J_ASSERT(is_open(), "File must be open.");
    while (num_bytes) {
      u32_t written = write(from, num_bytes);
      num_bytes -= written;
      from += num_bytes;
    }
  }

  ofile::~ofile() {
  }
}
