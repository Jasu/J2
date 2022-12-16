#pragma once

#include "hzd/maybe.hpp"
#include "mem/buffer.hpp"

namespace j::files::inline fds {
  struct fd;

  using io_result = maybe<i32_t, 0LL>;

  io_result read(const fd & file, void * J_NOT_NULL buf, u32_t bytes);

  io_result write(const fd & file, const void * J_NOT_NULL buf, u32_t bytes);

  mem::buffer read_to_end(const fd & file, u32_t num_zero_terminators = 0U);
}
