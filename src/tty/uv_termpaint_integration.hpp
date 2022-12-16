#pragma once

#include <termpaint.h>

#include "tty/common.hpp"
#include "tty/termpaint_to_keysym.hpp"
#include "input/modifier.hpp"
#include "strings/string_view.hpp"
#include "mem/shared_ptr.hpp"
#include "signals/signal.hpp"

namespace j::events {
  struct event_loop;
  struct signal_handle;
  struct stream_handle;
  struct prepare_handle;
  struct error_event;
  struct read_event;
  struct signal_event;
  struct prepare_event;
}
namespace j::tty {
  struct tty_restore_data;

  enum class uv_termpaint_status : u8_t {
    none,
    initial,
    integration_ready,
    terminal_attached,
    terminal_ready,
    max_status = terminal_ready,
  };

  enum class uv_termpaint_task : u8_t {
    none,
    auto_detect_wait,
    max_task = auto_detect_wait,
  };

  struct uv_termpaint_integration final {
    J_BOILERPLATE(uv_termpaint_integration, COPY_DEL)


    explicit uv_termpaint_integration(mem::shared_ptr<events::event_loop> event_loop,
                                      int fd = 0);

    ~uv_termpaint_integration();

    void on_input(const events::read_event & in);
    void on_signal(const events::signal_event & sig);
    void on_error(const events::error_event & err);
    void on_prepare(const events::prepare_event &);


    J_ALWAYS_INLINE void initialize() {
      initialize_to(uv_termpaint_status::terminal_ready);
    }
    void initialize_to(uv_termpaint_status status);

    J_ALWAYS_INLINE void finalize() {
      finalize_to(uv_termpaint_status::initial);
    }
    void finalize_to(uv_termpaint_status status);
    void apply_status_transition();
    void set_current_status(uv_termpaint_status status);

    void initialize_integration();
    void create_terminal();
    void initialize_terminal();

    void finalize_terminal();
    void abort_terminal_init();
    void detach_terminal();
    void finalize_integration();

    void start_task(uv_termpaint_task task);
    void check_task();
    void finish_task(uv_termpaint_status next_status);

    void task_auto_detect_wait();
    void task_auto_detect_finish();
    void update_size();

    // Called by termpaint when to print data on screen.
    void write_callback(const char * data, int length);
    // Called by termpaint to deallocate the integration.
    void free_callback();
    /// Called by termpaint with high-level events.
    void event_callback(termpaint_event * J_NOT_NULL event);
    // Called by termpaint when modifying terminal settings, to be able to restore the terminal.
    void restore_sequence_callback(const char * data, int length);

    void set_root(struct component * c) noexcept;

    void set_bracketed_paste_enabled(bool value) noexcept;

    /// The "base class" of the integration.
    termpaint_integration base;

    /// The terminal connected to the integration.
    termpaint_terminal * term = nullptr;
    /// The primary surface of the TTY.
    termpaint_surface * surf = nullptr;

    /// Handle to a libuv TTY (containing both input and output)
    events::stream_handle * tty_handle = nullptr;
    /// Terminal size, updated via SIGWINCH.
    size size;
    const atom_to_keysym_t & atom_to_keysym;

    struct component * root = nullptr;
    termpaint_text_measurement * measurement = nullptr;

    signals::signal<void (strings::const_string_view, input::modifier_mask)> on_char;
    signals::signal<void (u32_t keysym, input::modifier_mask)> on_key;
    signals::signal<void (strings::const_string_view)> on_paste;

    /// Initialization status of our integration
    uv_termpaint_status current_status = uv_termpaint_status::initial;
    /// Target status of the integration.
    uv_termpaint_status target_status = uv_termpaint_status::initial;
    /// Current task being performed / waited for.
    uv_termpaint_task current_task = uv_termpaint_task::none;

    /// The file descrpitor configured.
    int fd = -1;

    /// SIGWINCH handler, i.e. resize events.
    events::signal_handle * sigwinch_handle = nullptr;
    events::prepare_handle * prepare_handle = nullptr;
    /// Pointer to keep the event loop with us.
    mem::shared_ptr<events::event_loop> event_loop;

    tty_restore_data * restore_data = nullptr;
  };
}
