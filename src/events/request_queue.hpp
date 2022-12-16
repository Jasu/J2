#pragma once

#include "mem/ring_buffer.hpp"
#include "containers/vector.hpp"
#include "events/events.hpp"

extern "C" {
  typedef struct uv_handle_s uv_handle_t;
}

namespace j::events {
  struct request;
  struct write_request;
  struct write_overflow;

  struct request_queue final {
    request_queue(u32_t write_buffer_size);
    ~request_queue();

    void write(j::mem::const_memory_region data, write_handler on_write = {}, error_handler on_error = {});
    void flush(uv_handle_t * J_NOT_NULL handle);
    void on_finish(request * J_NOT_NULL req);

    mem::ring_buffer write_buffer;
    vector<request*> queue;
    write_overflow * first_overflow = nullptr;
    write_overflow * last_overflow = nullptr;
    u32_t num_executing = 0U;
    u32_t write_size = 0U;

    J_INLINE_GETTER bool has_pending_writes() const noexcept {
      return write_size || first_overflow;
    }
  private:
    write_request * last_write() noexcept;
    void do_add_write(write_handler on_write, error_handler on_error);
  };
}
