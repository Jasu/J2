#include "events/stream_handle.hpp"
#include "events/request_queue.hpp"
#include "logging/global.hpp"
#include "events/assert.hpp"

#include <uv.h>

namespace j::events {
  namespace {
    struct J_TYPE_HIDDEN stream_privates final {
      mem::ring_buffer rd_buffer;
      request_queue wr_queue;
      stream_privates()
        : rd_buffer(16 * 1024),
          wr_queue(16 * 1024)
      { }
    };

    J_INLINE_GETTER mem::ring_buffer & rd_buffer(stream_handle * J_NOT_NULL h) {
      return reinterpret_cast<stream_privates*>(h + 1)->rd_buffer;
    }

    J_INLINE_GETTER request_queue & wr_queue(stream_handle * J_NOT_NULL h) {
      return reinterpret_cast<stream_privates*>(h + 1)->wr_queue;
    }

    void stream_alloc_callback(uv_handle_t * J_NOT_NULL stream, sz_t suggested, uv_buf_t * J_NOT_NULL buf) {
      J_ASSUME_NOT_NULL(stream->data);
      auto & ring_buffer = rd_buffer(reinterpret_cast<base_handle*>(stream->data)->as_stream_handle());
      if (auto b = ring_buffer.get_write_buffer(min(suggested, 2048))) {
        buf->base = b.begin();
        buf->len = b.size();
      }
    }

    void stream_read_callback(uv_stream_t * J_NOT_NULL stream, ssz_t nread, const uv_buf_t * J_NOT_NULL buf) {
      J_ASSUME_NOT_NULL(stream->data);
      auto handle = reinterpret_cast<base_handle*>(stream->data)->as_stream_handle();
      if (nread != UV_EOF && nread < 0) {
        rd_buffer(handle).mark_as_read(buf->len);
        handle->trigger(error_event(nread));
        return;
      }

      J_ASSUME_NOT_NULL(buf->base, buf->len);
      if (nread) {
        if (nread == UV_EOF) {
          nread = 0;
        }
        J_ASSERT((sz_t)nread <= buf->len);
        handle->trigger(read_event{handle, mem::const_memory_region{buf->base, (u32_t)nread}});
        if (!nread) {
          if (handle->eof_handling == eof_mode::close) {
            handle->close();
          } else {
            handle->stop();
          }
        }
      }
      rd_buffer(handle).mark_as_read(buf->len);
    }
  }

  void stream_handle::write(j::mem::const_memory_region buf, write_handler on_write, error_handler on_error) {
    assert_alive();
    auto & q = wr_queue(this);
    if (buf.size() > 64U && !q.has_pending_writes()) {
      uv_buf_t b = {
        .base = const_cast<char*>(buf.begin()),
        .len = buf.size(),
      };
      i32_t written = uv_try_write((uv_stream_t*)handle(), &b, 1);
      if (written > 0) {
        if ((sz_t)written == buf.size()) {
          if (on_write) {
            on_write({this});
          }
          return;
        }
        J_ASSERT((sz_t)written < buf.size());
        buf.take_prefix(written);
      } else if (written != UV_EAGAIN) {
        auto err = error_event(written);
        if (on_error) {
          on_error(err);
        }
        trigger(err);
        return;
      }
    }
    q.write(buf, on_write, on_error);
  }

  void stream_handle::write(strings::const_string_view buf, write_handler on_write, error_handler on_error) {
    assert_alive();
    write(mem::const_memory_region(buf.begin(), buf.size()), on_write, on_error);
  }

  void stream_handle::flush() {
    assert_alive();
    wr_queue(this).flush(handle());
  }

  void stream_handle::start() {
    assert_alive();
    J_UV_CALL_CHECKED(uv_read_start, (uv_stream_t*)handle(), &stream_alloc_callback, &stream_read_callback);
    state = handle_state::running;
  }

  geometry::vec2i32 stream_handle::tty_size() noexcept {
    J_ASSERT(is_tty());
    geometry::vec2i32 result{-1, -1};
    J_UV_CALL_CHECKED(uv_tty_get_winsize, (uv_tty_t*)handle(), &result.x, &result.y);
    return result;
  }

  void stream_handle::stop() {
    J_UV_CALL_CHECKED(uv_read_stop, (uv_stream_t*)handle());
    state = handle_state::ready;
  }

  void stream_handle::on_finish(request * J_NOT_NULL req) {
    assert_alive();
    wr_queue(this).on_finish(req);
  }

  J_RETURNS_NONNULL stream_handle * stream_handle::create(uv_loop_t * J_NOT_NULL loop, int fd, eof_mode eof_handling) {
    void * data = j::allocate(sizeof(uv_tty_t) + sizeof(stream_handle) + sizeof(stream_privates));
    data = add_bytes(data, sizeof(uv_tty_t));
    return ::new (data) stream_handle(loop, fd, eof_handling);
  }

  stream_handle::stream_handle(uv_loop_t * J_NOT_NULL loop, int fd, eof_mode eof_handling)
    : basic_handle(handle_type::tty, sizeof(uv_tty_t), fd),
      eof_handling(eof_handling)
  {
    J_ASSUME(fd >= 0);
    uv_tty_t * h = (uv_tty_t*)handle();
    J_UV_CALL_CHECKED(uv_tty_init, loop, h, fd, 0);
    h->data = (base_handle*)this;
    ::new (this + 1) stream_privates();
  }

  stream_handle::~stream_handle() {
    ((stream_privates*)(this + 1))->~stream_privates();
  }

  void stream_handle::prepare() {
    bool has = has_handlers<read_event>();
    if (has && is_ready()) {
      start();
    } else if (!has && is_running()) {
      stop();
    }
  }
}
