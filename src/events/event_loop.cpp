#include "events/event_loop.hpp"
#include "events/assert.hpp"
#include "events/prepare_handle.hpp"
#include "services/service_definition.hpp"
#include "events/stream_handle.hpp"
#include "events/poll_handle.hpp"
#include "events/signal_handle.hpp"
#include "containers/hash_map.hpp"
#include "containers/vector.hpp"

#include <uv.h>

namespace j::events {
  error_event::error_event(int code) noexcept
    : code(code),
      title(uv_err_name(code)),
      description(uv_strerror(code))
  {
  }

  namespace {
    struct J_TYPE_HIDDEN event_handle_key final {
      handle_type type;
      int fd;

      J_INLINE_GETTER bool operator==(const event_handle_key &) const noexcept = default;
    };

    struct J_TYPE_HIDDEN event_handle_key_hash final {
      u32_t operator()(const event_handle_key & k) const noexcept {
        return crc32(12, (((u32_t)k.type << 28) ^ (u32_t)k.fd));
      }
    };

    using map_t = hash_map<event_handle_key, base_handle*, event_handle_key_hash>;
    using stream_handles_t = vector<stream_handle*>;

    struct J_TYPE_HIDDEN event_loop_private final {
      map_t map;
      uv_loop_t loop;
      stream_handles_t stream_handles;
      uv_prepare_t prepare_handle;
    };

    template<typename T>
    J_INLINE_GETTER auto privates(T * J_NOT_NULL loop) noexcept {
      return (event_loop_private*)(loop + 1);
    }

    template<typename T>
    J_INLINE_GETTER auto & map(T * J_NOT_NULL loop) noexcept {
      return privates(loop)->map;
    }

    template<typename T>
    J_INLINE_GETTER auto & stream_handles(T * J_NOT_NULL loop) noexcept {
      return privates(loop)->stream_handles;
    }

    J_INLINE_GETTER uv_loop_t * J_NOT_NULL loop(const event_loop * J_NOT_NULL loop) noexcept {
      return &privates(const_cast<event_loop*>(loop))->loop;
    }

    J_INLINE_GETTER uv_prepare_t * J_NOT_NULL get_prepare_handle(event_loop * J_NOT_NULL loop) noexcept {
      return &privates(loop)->prepare_handle;
    }

    void prepare(uv_prepare_t * J_NOT_NULL prepare) {
      J_ASSUME_NOT_NULL(prepare->loop->data);
      event_loop * l = (event_loop*)(prepare->loop->data);
      for (auto it : stream_handles(l)) {
        if (it->state == handle_state::running) {
          it->flush();
        }
      }
      for (auto it : map(l)) {
        it.second->visit([](auto * h) {
          h->prepare();
        });
      }
    }
  }

  event_loop::event_loop() {
    ::new (privates(this)) event_loop_private();
    auto l = loop(this);
    J_UV_CALL_CHECKED(uv_loop_init, l);
    uv_loop_set_data(l, this);
  }

  event_loop::~event_loop() {
    J_ASSERT(map(this).empty());
    auto l = loop(this);
    if (uv_loop_alive(l)) {
      cycle();
      J_ASSERT(!uv_loop_alive(l), "Could not release loop resources.");
    }
    J_UV_CALL_CHECKED(uv_loop_close, l);
    privates(this)->~event_loop_private();
  }

  void event_loop::remove_poll_handle(int fd) {
    J_ASSUME(fd >= 0);
    auto & m = map(this);
    event_handle_key key{handle_type::poll, fd};
    auto it = m.find(key);
    J_ASSERT(it != m.end());
    it->second->close();
    m.erase(it);
  }

  poll_handle * event_loop::add_poll_handle(int fd, poll_flags flags) {
    J_ASSUME(fd >= 0);
    J_ASSERT(flags, "Poll handler has a zero mask.");
    event_handle_key key{handle_type::poll, fd};
    auto p = map(this).emplace(key, nullptr);
    if (p.second) {
      p.first->second = poll_handle::create(loop(this), fd, flags);
    }
    return p.first->second->as_poll_handle();
  }

  signal_handle * event_loop::add_signal_handle(int signum) {
    J_ASSUME(signum >= 0);
    event_handle_key key{handle_type::signal, signum};
    auto p = map(this).emplace(key, nullptr);
    if (p.second) {
      p.first->second = signal_handle::create(loop(this), signum);
    }
    return p.first->second->as_signal_handle();
  }

  prepare_handle * event_loop::add_prepare_handle() {
    return prepare_handle::create(loop(this));
  }

  stream_handle * event_loop::add_tty_handle(int fd) {
    J_ASSUME(fd >= 0);
    event_handle_key key{handle_type::tty, fd};
    auto p = map(this).emplace(key, nullptr);
    if (p.second) {
      p.first->second = stream_handle::create(loop(this), fd, eof_mode::close);
      stream_handles(this).push_back((stream_handle*)p.first->second);
    }
    return p.first->second->as_stream_handle();
  }

  poll_handle * event_loop::find_poll_handle(int fd) {
    J_ASSUME(fd >= 0);
    auto it = map(this).maybe_at({ handle_type::poll, fd });
    return it ? (*it)->as_poll_handle() : nullptr;
  }

  stream_handle * event_loop::find_stream_handle(int fd) {
    J_ASSUME(fd >= 0);
    auto it = map(this).maybe_at({ handle_type::tty, fd });
    return it ? (*it)->as_stream_handle() : nullptr;
  }

  signal_handle * event_loop::find_signal_handle(int signum) {
    J_ASSUME(signum >= 0);
    auto it = map(this).maybe_at({ handle_type::signal, signum });
    return it ? (*it)->as_signal_handle() : nullptr;
  }

  void event_loop::remove_tty_handle(int fd) {
    J_ASSUME(fd >= 0);
    auto & m = map(this);
    event_handle_key key{handle_type::tty, fd};
    auto it = m.find(key);
    J_ASSERT(it != m.end());
    auto & handles = stream_handles(this);
    for (auto it2 = handles.begin(), end = handles.end(); it2 != end; ++it2) {
      if (*it2 == it->second) {
        handles.erase(it2);
        break;
      }
    }
    ((poll_handle*)(it->second))->close();
    m.erase(it);
  }

  void event_loop::run() {
    J_ASSUME(!is_running);
    is_running = true;

    auto ph = get_prepare_handle(this);
    J_UV_CALL_CHECKED(uv_prepare_init, loop(this), ph);
    uv_unref((uv_handle_t*)ph);
    J_UV_CALL_CHECKED(uv_prepare_start, get_prepare_handle(this), prepare);
    prepare(ph);

    uv_run(loop(this), uv_run_mode::UV_RUN_DEFAULT);
    uv_close((uv_handle_t*)ph, nullptr);
    is_running = false;
  }

  void event_loop::stop() {
    J_ASSUME(is_running);
    uv_stop(loop(this));
  }

  void event_loop::cycle() {
    J_ASSUME(!is_running);
    is_running = true;
    uv_run(loop(this), uv_run_mode::UV_RUN_ONCE);
    is_running = false;
  }

  void event_loop::on_before_delete(base_handle * J_NOT_NULL handle) {
    event_handle_key key{handle->type, handle->fd};
    auto it = map(this).find(key);
    if (it == map(this).end()) {
      return;
    }
    map(this).erase(it);
    if (handle->type == handle_type::tty) {
      auto & handles = stream_handles(this);
      for (auto it2 = handles.begin(), end = handles.end(); it2 != end; ++it2) {
        if (*it2 == it->second) {
          handles.erase(it2);
          break;
        }
      }
    }
  }

  J_RETURNS_NONNULL uv_loop_t * event_loop::uv_loop() noexcept {
    return loop(this);
  }

  namespace {
    j::mem::shared_ptr<event_loop> create_event_loop() {
      return j::mem::make_shared_sized<event_loop>(sizeof(event_loop) + sizeof(event_loop_private));
    }

    using namespace services;
    J_A(ND, NODESTROY) const service_definition<event_loop> def(
      "events.event_loop",
      "UV event loop",
      services::create = services::factory(create_event_loop)
    );
  }
}
