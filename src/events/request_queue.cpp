#include "events/request_queue.hpp"
#include "logging/global.hpp"
#include "events/requests.hpp"

namespace j::events {
  request_queue::request_queue(u32_t write_buffer_size)
    : write_buffer(write_buffer_size)
  {
  }

  request_queue::~request_queue() {
    J_ASSUME(!num_executing);
  }
  void request_queue::do_add_write(write_handler on_write, error_handler on_error) {
    if (write_request * prev = last_write()) {
      if (on_write || on_error) {
        queue.back() = write_request::add_handlers(prev, on_write, on_error);
      }
    } else {
      queue.push_back(write_request::create(on_write, on_error));
    }
  }

  void request_queue::write(j::mem::const_memory_region data, write_handler on_write, error_handler on_error) {
    J_ASSERT(data);
    if (!last_overflow && write_buffer.writable_size() >= data.size()) {
      u32_t written = write_buffer.write(data);
      do_add_write(on_write, on_error);
      write_size += written;
      data.take_prefix(written);
    }
    if (data) {
      auto over = write_overflow::create(data, on_write, on_error);
      if (last_overflow) {
        last_overflow->next = over;
      } else {
        first_overflow = last_overflow = over;
      }
    }
  }

  void request_queue::flush(uv_handle_t * J_NOT_NULL handle) {
    write_buffer.dump("Write requests");
    for (request * req : queue) {
      J_ASSUME_NOT_NULL(req);
      switch (req->type) {
      case request_type::write: {
        write_request * wr = (write_request*)req;
        wr->initialize(write_buffer.get_read_buffers(write_buffer.readable_size()));
        J_ASSERT(write_size <= wr->size());
        write_size -= wr->size();
        ++num_executing;
        wr->execute((uv_stream_t*)handle);
        break;
      }
      }
    }
    queue.clear(queue.capacity() > 64U);
  }

  void request_queue::on_finish(request * J_NOT_NULL req) {
    J_ASSUME(num_executing > 0);
    --num_executing;
    if (req->type != request_type::write) {
      return;
    }
    while (first_overflow) {
      auto buf = first_overflow->buffer();
      J_ASSERT(buf);
      auto written = write_buffer.write(buf);
      J_ASSUME(written);
      do_add_write(first_overflow->on_write, first_overflow->on_error);
      write_size += written;
      if (buf.size() > written) {
        first_overflow->skip += written;
        break;
      } else {
        J_ASSERT(buf.size() == written);
        auto next = first_overflow->next;
        j::free(first_overflow);
        first_overflow = next;
        if (!first_overflow) {
          last_overflow = nullptr;
        }
      }
    }
  }

  write_request * request_queue::last_write() noexcept {
    if (!queue) {
      return nullptr;
    }
    request * req = queue.back();
    J_ASSUME(req);
    return req->type == request_type::write ? (write_request*)req : nullptr;
  }
}
