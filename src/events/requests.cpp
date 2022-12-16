#include "events/requests.hpp"
#include "events/assert.hpp"
#include "logging/global.hpp"
#include "events/stream_handle.hpp"

namespace j::events {
  namespace {
    struct J_TYPE_HIDDEN write_handlers final {
      write_handler on_write;
      error_handler on_error;
    };

    void write_callback(uv_write_t * J_NOT_NULL req, int status) {
      J_ASSUME(req->data);
      J_ASSUME_NOT_NULL(req->handle->data);
      auto handle = ((base_handle*)req->handle->data)->as_stream_handle();

      write_request * wr = (write_request*)req->data;
      error_event err;
      if (J_UNLIKELY(!status)) {
        err = error_event(status);
      }

      write_event ev{handle};
      for (write_handlers * it = (write_handlers*)(wr + 1), *end = it + wr->num_handlers;
           it != end; ++it) {
        if (J_UNLIKELY(!status) && it->on_write) {
          it->on_write(ev);
        } else if (J_LIKELY(status) && it->on_error) {
          it->on_error(err);
        }
      }

      handle->on_finish(wr);

      wr->~write_request();
      j::free(wr);
    }
  }

  J_RETURNS_NONNULL write_request * write_request::add_handlers(write_request * J_NOT_NULL wr, write_handler on_write, error_handler on_error) {
    J_ASSERT(on_write || on_error);
    u32_t size = sizeof(write_request) + (wr->num_handlers + 1U) * sizeof(write_handlers);
    wr = (write_request*)j::reallocate(wr, size);
    ((write_handlers*)(wr + 1))[wr->num_handlers++] = { on_write, on_error };
    return wr;
  }
  void write_request::initialize(j::pair<j::mem::memory_region, j::mem::memory_region> buffers) noexcept {
    J_DEBUG("Bufs {} {}", buffers.first.size(), buffers.second.size());
    J_ASSERT(buffers.first);
    bufs[0] = uv_buf_init(buffers.first.begin(), buffers.first.size());
    bufs[1] = uv_buf_init(buffers.second.begin(), buffers.second.size());
  }

  void write_request::execute(uv_stream_t * J_NOT_NULL stream) {
    req.data = this;
    J_ASSUME_NOT_NULL(bufs[0].base, bufs[0].len);
    J_UV_CALL_CHECKED(uv_write, &req, stream, bufs, bufs[1].len ? 2 : 1, write_callback);
  }

  J_RETURNS_NONNULL write_request * write_request::create(write_handler on_write, error_handler on_error) {
    u32_t size = sizeof(write_request);
    if (on_write || on_error) {
      size += sizeof(write_handlers);
    }
    return ::new (::j::allocate(size)) write_request(on_write, on_error);
  }

  write_request::write_request(write_handler on_write, error_handler on_error) noexcept
    : request{request_type::write},
      num_handlers(on_write || on_error)
  {
    j::memzero(bufs, sizeof(uv_buf_t) * 2);
    if (on_write || on_error) {
      *reinterpret_cast<write_handlers*>(this + 1) = { on_write, on_error };
    }
  }

  J_RETURNS_NONNULL write_overflow * write_overflow::create(j::mem::const_memory_region data, write_handler on_write, error_handler on_error) {
    J_ASSERT(data);
    u32_t size = sizeof(write_overflow) + data.size();
    return ::new (::j::allocate(size)) write_overflow(data, on_write, on_error);
  }

  write_overflow::write_overflow(j::mem::const_memory_region data, write_handler on_write, error_handler on_error) noexcept
      : on_write(on_write),
        on_error(on_error),
        size(data.size())
  {
    J_ASSERT(data);
    data.copy_to(this + 1);
  }
}
