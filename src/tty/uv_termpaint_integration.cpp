#include "tty/uv_termpaint_integration.hpp"
#include "tty/tty_restore_data.hpp"
#include "tty/component.hpp"
#include "tty/tty_restore.hpp"
#include "logging/global.hpp"
#include "events/signal_handle.hpp"
#include "events/stream_handle.hpp"
#include "events/prepare_handle.hpp"
#include "events/event_loop.hpp"

#include <termpaint.h>
#include <unistd.h>
#include <signal.h>

namespace j::tty {
  namespace {
    const u8_t num_statuses = (u8_t)uv_termpaint_status::max_status + 1U;
    const u8_t num_tasks = (u8_t)uv_termpaint_task::max_task + 1U;

    using transition_fn = void (uv_termpaint_integration::*)();

#define J_MM__ 0
#define J_MM_S (1 << (u8_t)input::modifier::shift)
#define J_MM_C (1 << (u8_t)input::modifier::ctrl)
#define J_MM_A (1 << (u8_t)input::modifier::alt)
#define J_MM_G (1 << (u8_t)input::modifier::alt_gr)

#define J_TM__ 0
#define J_TM_S TERMPAINT_MOD_SHIFT
#define J_TM_C TERMPAINT_MOD_CTRL
#define J_TM_A TERMPAINT_MOD_ALT
#define J_TM_G TERMPAINT_MOD_ALTGR

#define J_MOD(S, C, A, G) [J_TM_##S + J_TM_##C + J_TM_##A + J_TM_##G] \
    = input::modifier_mask(J_MM_##S + J_MM_##C + J_MM_##A + J_MM_##G)

    const input::modifier_mask modmasks[16]{
      J_MOD(_, _, _, _),
        J_MOD(S, _, _, _),
        J_MOD(_, C, _, _),
        J_MOD(S, C, _, _),
        J_MOD(_, _, A, _),
        J_MOD(S, _, A, _),
        J_MOD(_, C, A, _),
        J_MOD(S, C, A, _),
        J_MOD(_, _, _, G),
        J_MOD(S, _, _, G),
        J_MOD(_, C, _, G),
        J_MOD(S, C, _, G),
        J_MOD(_, _, A, G),
        J_MOD(S, _, A, G),
        J_MOD(_, C, A, G),
        J_MOD(S, C, A, G),
        };


#define J_EV(N) [TERMPAINT_EV_##N] = "EV_"#N
    const char * const event_names[]{
      J_EV(UNKNOWN),
        J_EV(CHAR),
        J_EV(KEY),
        J_EV(AUTO_DETECT_FINISHED),
        J_EV(OVERFLOW),
        J_EV(INVALID_UTF8),
        J_EV(CURSOR_POSITION),
        J_EV(MODE_REPORT),
        J_EV(COLOR_SLOT_REPORT),
        J_EV(REPAINT_REQUESTED),
        J_EV(MOUSE),
        J_EV(MISC),
        J_EV(PALETTE_COLOR_REPORT),
        J_EV(PASTE),
        J_EV(RAW_PRI_DEV_ATTRIB),
        J_EV(RAW_SEC_DEV_ATTRIB),
        J_EV(RAW_3RD_DEV_ATTRIB),
        J_EV(RAW_DECREQTPARM),
        J_EV(RAW_TERM_NAME),
        J_EV(RAW_TERMINFO_QUERY_REPLY),
        };
#undef J_EV

    constexpr transition_fn task_fns[num_tasks]{
      [(u8_t)uv_termpaint_task::none] = nullptr,
        [(u8_t)uv_termpaint_task::auto_detect_wait] = &uv_termpaint_integration::task_auto_detect_wait,
        };

    constexpr transition_fn finalize_transitions[num_statuses]{
      [(u8_t)uv_termpaint_status::none] = nullptr,
        [(u8_t)uv_termpaint_status::initial] = nullptr,
        [(u8_t)uv_termpaint_status::integration_ready] = &uv_termpaint_integration::finalize_integration,
        [(u8_t)uv_termpaint_status::terminal_attached] = &uv_termpaint_integration::detach_terminal,
        [(u8_t)uv_termpaint_status::terminal_ready] = &uv_termpaint_integration::finalize_terminal,
        };

    constexpr transition_fn initialize_transitions[num_statuses]{
      [(u8_t)uv_termpaint_status::none] = nullptr,
        [(u8_t)uv_termpaint_status::initial] = &uv_termpaint_integration::initialize_integration,
        [(u8_t)uv_termpaint_status::integration_ready] = &uv_termpaint_integration::create_terminal,
        [(u8_t)uv_termpaint_status::terminal_attached] = &uv_termpaint_integration::initialize_terminal,
        [(u8_t)uv_termpaint_status::terminal_ready] = nullptr,
        };

    J_INLINE_GETTER_NONNULL uv_termpaint_integration * uv_int(termpaint_integration * i) noexcept {
      J_ASSUME_NOT_NULL(i);
      return J_CONTAINER_OF(i, uv_termpaint_integration, base);
    }

    void write_callback_static(termpaint_integration * i, const char * data, int sz) {
      auto u = uv_int(i);
      J_ASSUME(u->current_status >= uv_termpaint_status::terminal_attached);
      if (!data || !sz) {
        return;
      }
      u->tty_handle->write(j::mem::const_memory_region(data, sz));
    }

    void flush_callback_static(termpaint_integration * i) {
      auto u = uv_int(i);
      J_ASSUME(u->current_status >= uv_termpaint_status::terminal_attached);
      u->tty_handle->flush();
    }

    void free_callback_static(termpaint_integration * i) {
      uv_int(i)->free_callback();
    }

    void log_callback_static(termpaint_integration *, const char * data, int size) {
      if (size && data[size - 1] == '\n') {
        --size;
      }
      J_DEBUG("TERMPAINT: {}", strings::const_string_view(data, size));
    }

    void event_callback_static(void * userdata, termpaint_event * event) {
      J_ASSUME_NOT_NULL(userdata, event);
      ((uv_termpaint_integration*)(userdata))->event_callback(event);
    }
    void restore_sequence_callback_static(termpaint_integration * i, const char * data, int size) {
      J_ASSUME_NOT_NULL(data, size);
      uv_int(i)->restore_sequence_callback(data, size);
    }
  }

  uv_termpaint_integration::uv_termpaint_integration(mem::shared_ptr<events::event_loop> event_loop, int fd)
    : atom_to_keysym(get_termpaint_to_keysym()),
      fd(fd),
      event_loop(static_cast<mem::shared_ptr<events::event_loop> &&>(event_loop))
  {
    J_ASSUME(fd >= 0);
    J_ASSERT(this->event_loop);
    restore_data = get_tty_restore_data();
    if (tcgetattr(fd, &restore_data->termios) < 0) {
      J_FAIL("Could not get termios attributes.");
    }
    restore_data->has_termios = true;
    restore_data->fd = fd;
  }

  void uv_termpaint_integration::initialize_to(uv_termpaint_status status) {
    target_status = max(current_status, status);
    apply_status_transition();
  }

  void uv_termpaint_integration::finalize_to(uv_termpaint_status status) {
    target_status = min(current_status, status);
    apply_status_transition();
  }

  void uv_termpaint_integration::apply_status_transition() {
    transition_fn fn = nullptr;
    if (target_status < current_status) {
      fn = finalize_transitions[(u8_t)current_status];
    } else if (target_status > current_status) {
      fn = initialize_transitions[(u8_t)current_status];
    }
    if (fn) {
      (this->*fn)();
    }
  }

  void uv_termpaint_integration::finish_task(uv_termpaint_status next_status) {
    J_ASSUME(current_task != uv_termpaint_task::none);
    current_task = uv_termpaint_task::none;
    if (next_status != uv_termpaint_status::none) {
      set_current_status(next_status);
    }
  }

  void uv_termpaint_integration::set_current_status(uv_termpaint_status status) {
    current_task = uv_termpaint_task::none;
    if (current_status == status) {
      return;
    }
    current_status = status;
    if (current_status != target_status) {
      apply_status_transition();
    }
  }

  void uv_termpaint_integration::initialize_integration() {
    J_ASSUME(current_status == uv_termpaint_status::initial);
    J_ASSUME(target_status > uv_termpaint_status::initial);
    termpaint_integration_init(&base, &free_callback_static, &write_callback_static, &flush_callback_static);
    termpaint_integration_set_logging_func(&base, log_callback_static);
    set_current_status(uv_termpaint_status::integration_ready);
  }

  void uv_termpaint_integration::create_terminal() {
    J_ASSUME(current_status == uv_termpaint_status::integration_ready);
    J_ASSUME(target_status > uv_termpaint_status::integration_ready);
    J_ASSUME(!tty_handle);
    J_ASSUME(!term);
    tty_handle = this->event_loop->add_tty_handle(fd);

    sigwinch_handle = this->event_loop->add_signal_handle(SIGINT);
    sigwinch_handle->on(events::signal_handler(this, &uv_termpaint_integration::on_signal));
    sigwinch_handle->start();

    mark_tty_dirty();
    ::termios new_termios;
    if (tcgetattr(fd, &new_termios) < 0) {
      J_FAIL("Could not get termios attributes.");
    }
    new_termios.c_iflag |= IGNBRK|IGNPAR;
    new_termios.c_iflag &= ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    new_termios.c_oflag &= ~(OPOST|ONLCR|OCRNL|ONOCR|ONLRET);
    new_termios.c_lflag &= ~(ICANON|IEXTEN|ECHO);
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    new_termios.c_cc[VSUSP] = 0;
    if (tcsetattr (fd, TCSAFLUSH, &new_termios) < 0) {
      J_FAIL("Setting termios failed");
    }

    term = termpaint_terminal_new(&base);
    termpaint_terminal_set_log_mask(term, TERMPAINT_LOG_AUTO_DETECT_TRACE);
    tty_handle->on(events::read_handler(this, &uv_termpaint_integration::on_input));
    tty_handle->start();

    sigwinch_handle = this->event_loop->add_signal_handle(SIGWINCH);
    sigwinch_handle->on(events::signal_handler(this, &uv_termpaint_integration::on_signal));
    sigwinch_handle->start();

    prepare_handle = this->event_loop->add_prepare_handle();
    prepare_handle->on(events::prepare_handler(this, &uv_termpaint_integration::on_prepare));
    prepare_handle->start();

    set_current_status(uv_termpaint_status::terminal_attached);
  }

  void uv_termpaint_integration::initialize_terminal() {
    J_ASSUME(current_status == uv_termpaint_status::terminal_attached);
    J_ASSUME(target_status > uv_termpaint_status::terminal_attached);
    J_ASSUME_NOT_NULL(term, tty_handle);
    J_ASSUME(tty_handle->state == events::handle_state::running);
    termpaint_terminal_set_event_cb(term, event_callback_static, this);
    termpaint_terminal_auto_detect(term);
    start_task(uv_termpaint_task::auto_detect_wait);
  }

  void uv_termpaint_integration::finalize_terminal() {
    J_ASSUME(current_status == uv_termpaint_status::terminal_ready);
    J_ASSUME(target_status < uv_termpaint_status::terminal_ready);
    termpaint_text_measurement_free(measurement);
    measurement = nullptr;
    // termpaint_surface_free(surf);
    surf = nullptr;
    termpaint_terminal_set_event_cb(term, nullptr, nullptr);
    set_current_status(uv_termpaint_status::terminal_attached);
  }

  void uv_termpaint_integration::detach_terminal() {
    J_ASSUME(current_status == uv_termpaint_status::terminal_attached);
    J_ASSUME(target_status < uv_termpaint_status::terminal_attached);
    J_ASSUME(tty_handle);
    J_ASSUME(term);

    sigwinch_handle->stop();
    tty_handle->stop();
    sigwinch_handle->off(events::signal_handler(this, &uv_termpaint_integration::on_signal));
    tty_handle->off(events::read_handler(this, &uv_termpaint_integration::on_input));
    sigwinch_handle->close();
    tty_handle->close();
    sigwinch_handle = nullptr;
    tty_handle = nullptr;


    termpaint_terminal_free(this->term);
    this->term = nullptr;
    set_current_status(uv_termpaint_status::integration_ready);
  }

  void uv_termpaint_integration::finalize_integration() {
    J_ASSUME(!this->tty_handle);
    J_ASSUME(!this->term);
    termpaint_integration_deinit(&base);
    set_current_status(uv_termpaint_status::initial);
  }

  void uv_termpaint_integration::start_task(uv_termpaint_task task) {
    J_ASSUME(current_task == uv_termpaint_task::none);
    J_ASSUME(task != uv_termpaint_task::none);
    current_task = task;
    check_task();
  }

  void uv_termpaint_integration::check_task() {
    J_ASSUME(current_task != uv_termpaint_task::none);
    auto task_fn = task_fns[(u8_t)current_task];
    J_ASSUME(task_fn);
    (this->*task_fn)();
  }

  void uv_termpaint_integration::task_auto_detect_wait() {
    J_ASSUME(current_status == uv_termpaint_status::terminal_attached);
    J_ASSUME_NOT_NULL(term, tty_handle);
    switch (termpaint_terminal_auto_detect_state(term)) {
    case termpaint_auto_detect_none:
      J_FAIL("Auto detect task was never started.");
    case termpaint_auto_detect_running:
      return;
    case termpaint_auto_detect_done:
      break;
    }
    task_auto_detect_finish();
  }

  void uv_termpaint_integration::task_auto_detect_finish() {
    J_ASSUME(current_status == uv_termpaint_status::terminal_attached);
    J_ASSUME_NOT_NULL(term, tty_handle);
    J_DEBUG("Auto detection finished.");
    termpaint_terminal_auto_detect_apply_input_quirks(term, restore_data->termios.c_cc[VERASE] == 0x08);

    update_size();
    termpaint_terminal_setup_fullscreen(term, size.width, size.height, "+kbdsig");

    termpaint_integration_set_restore_sequence_updated(&base, restore_sequence_callback_static);

    const char * seq = termpaint_terminal_restore_sequence(term);
    restore_data->restore_sequence_length = j::strlen(seq);
    j::memcpy(restore_data->restore_sequence, seq, restore_data->restore_sequence_length);
    mark_tty_dirty();

    surf = termpaint_terminal_get_surface(term);
    measurement = termpaint_text_measurement_new(surf);

    termpaint_terminal_handle_paste(term, true);
    termpaint_terminal_request_tagged_paste(term, true);

    finish_task(uv_termpaint_status::terminal_ready);
  }

  void uv_termpaint_integration::update_size() {
    J_ASSUME(current_status >= uv_termpaint_status::terminal_attached);
    auto new_size = tty_handle->tty_size();
    if (new_size.x != size.width || new_size.y != size.height) {
      size = { (i16_t)new_size.x, (i16_t)new_size.y };
    }
  }

  uv_termpaint_integration::~uv_termpaint_integration() {
    J_ASSUME(current_status == uv_termpaint_status::initial);
  }

  void uv_termpaint_integration::free_callback() {
    finalize();
  }

  void uv_termpaint_integration::event_callback(termpaint_event * J_NOT_NULL event) {
    J_ASSUME(current_status >= uv_termpaint_status::terminal_attached);
    switch (event->type) {
    case TERMPAINT_EV_INVALID_UTF8:
    case TERMPAINT_EV_CHAR:
      if (current_status == uv_termpaint_status::terminal_ready) {
        // J_DEBUG("  {} Str: {#bold}\"{}\"{/} {#light_gray}Mod: {}",
        //         event_names[event->type],
        //         strings::const_string_view(event->c.string, event->c.length),
        //         modmasks[(u8_t)event->c.modifier & 15]);
        on_char(strings::const_string_view(event->c.string, event->c.length),
                modmasks[(u8_t)event->c.modifier & 15]);
      }
      break;
    case TERMPAINT_EV_KEY:
      if (current_status == uv_termpaint_status::terminal_ready) {
        const u32_t * keysym = atom_to_keysym.maybe_at((const void*)event->key.atom);
        // J_DEBUG("  EV_KEY: Sym: {xkb-keysym} Mod: {}", keysym ? *keysym : 0U,
        //         modmasks[(u8_t)event->key.modifier & 15]);
        if (keysym) {
          on_key(*keysym, modmasks[(u8_t)event->key.modifier & 15]);
        }
      }
      break;
    case TERMPAINT_EV_PASTE:
      if (event->paste.length) {
        on_paste(strings::const_string_view(event->paste.string, event->paste.length));
      }
      break;
    case TERMPAINT_EV_AUTO_DETECT_FINISHED:
      if (current_task == uv_termpaint_task::auto_detect_wait) {
        uv_termpaint_integration::task_auto_detect_finish();
      }
      [[fallthrough]];
    default:
      J_DEBUG("Event {}", event_names[event->type]);
      break;
    }
  }

  void uv_termpaint_integration::restore_sequence_callback(const char * data, int length) {
    J_ASSUME_NOT_NULL(data, length);
    restore_data->restore_sequence_length = length;
    j::memcpy(restore_data->restore_sequence, data, length);
    mark_tty_dirty();
  }

  void uv_termpaint_integration::on_input(const events::read_event & in) {
    J_ASSUME(current_status >= uv_termpaint_status::terminal_attached);
    if (in.is_eof()) {
      J_FAIL("EOF from terminal");
    }
    termpaint_terminal_add_input_data(term, in.buffer.begin(), in.buffer.size());
    if (current_task != uv_termpaint_task::none) {
      check_task();
    }
  }

  void uv_termpaint_integration::on_signal(const events::signal_event & sig) {
    J_ASSERT(sig.signal == SIGWINCH);
    if (current_status >= uv_termpaint_status::terminal_attached) {
      update_size();
    }
  }

  void uv_termpaint_integration::on_prepare(const events::prepare_event &) {
    if (!root || (!root->is_dirty && !root->has_dirty_children && !root->is_layout_dirty) || current_status != uv_termpaint_status::terminal_ready) {
      return;
    }
    output_context ctx{this, root};
    if (root->is_layout_dirty) {
      root->resize(size);
      root->relayout(ctx);
      root->is_layout_dirty = false;
    }
    if (root->is_dirty || root->has_dirty_children) {
      root->render(ctx);
      root->is_dirty = false;
      root->has_dirty_children = false;
      termpaint_terminal_flush(term, false);
    }
  }

  void uv_termpaint_integration::on_error(const events::error_event & err) {
    J_DEBUG("Error {} ({}): {}", err.title, err.code, err.description);
    J_FAIL("Error from UV");
  }

  void uv_termpaint_integration::set_root(struct component * c) noexcept {
    root = c;
    if (root) {
      root->reposition({0, 0});
    }
  }

}
