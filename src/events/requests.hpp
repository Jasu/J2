#pragma once

#include <uv.h>
#include "events/events.hpp"
#include "containers/pair.hpp"

namespace j::events {
  enum class request_type : u8_t {
    write,
  };

  struct request {
    request_type type;
  };

  struct write_request final : request {
    uv_write_t req;
    uv_buf_t bufs[2];
    u32_t num_handlers = 0U;

    static J_RETURNS_NONNULL write_request * create(write_handler on_write = {}, error_handler on_error = {});

    static J_RETURNS_NONNULL write_request * add_handlers(write_request * J_NOT_NULL wr, write_handler on_write, error_handler on_error);

    void initialize(j::pair<j::mem::memory_region, j::mem::memory_region> buffers) noexcept;

    void execute(uv_stream_t * J_NOT_NULL stream);

    J_INLINE_GETTER u32_t size() const noexcept {
      return bufs[0].len + bufs[1].len;
    }

  private:
    write_request(write_handler on_write, error_handler on_error) noexcept;
  };

  struct write_overflow final : request {
    write_handler on_write;
    error_handler on_error;
    write_overflow * next = nullptr;
    u32_t size, skip = 0U;

    static J_RETURNS_NONNULL write_overflow * create(j::mem::const_memory_region data, write_handler on_write = {}, error_handler on_error = {});

    [[nodiscard]] j::mem::const_memory_region buffer() const noexcept {
      J_ASSUME(size > skip);
      return {(char*)(this + 1) + skip, size};
    }

  private:
    write_overflow(j::mem::const_memory_region data, write_handler on_write, error_handler on_error) noexcept;
  };
}
