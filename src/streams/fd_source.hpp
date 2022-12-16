#include "streams/source.hpp"

#include "mem/shared_ptr_fwd.hpp"
#include "files/fds/fd.hpp"

namespace j::streams {

  class fd_source : public source {
  public:
    explicit fd_source(const files::fd & fd);

    u32_t read(char * to, u32_t num_bytes) override final;
  private:
    files::fd m_fd;
  };
}
