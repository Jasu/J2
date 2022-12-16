#pragma once

#include "geometry/rect.hpp"
#include "signals/signal.hpp"

namespace j::input {
  struct key_event;
}

namespace j::windowing {
  namespace g = j::geometry;

  /// Base class for windows of different platforms.
  struct window {
    window(u16_t initial_width, u16_t initial_height) noexcept;

    virtual ~window();

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void destroy() = 0;

    virtual void request_close();

    /// Triggered when window close is requested by the user.
    signals::signal<bool ()> on_close_request;

    /// Triggered before the window is destroyed.
    signals::signal<void ()> on_before_destroy;
    /// Triggered after the window is destroyed.
    signals::signal<void ()> on_after_destroy;

    signals::signal<void (const input::key_event &)> on_key_press;
    signals::signal<void (const input::key_event &)> on_key_release;

    signals::signal<void (u8_t, g::vec2i16)> on_button_press;
    signals::signal<void (u8_t, g::vec2i16)> on_button_release;

    signals::signal<void (g::vec2i16)> on_mouse_move;
    signals::signal<void (g::vec2i16)> on_mouse_enter;
    signals::signal<void (g::vec2i16)> on_mouse_leave;

    signals::signal<void ()> on_show;
    signals::signal<void ()> on_hide;
    signals::signal<void (const g::rect_i16 &)> on_resize;

    window(window &&) = delete;

    u16_t width;
    u16_t height;
  };
}
