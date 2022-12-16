#pragma once

#include "hzd/types.hpp"

namespace j::windowing::x11 {
  /// ICCCM Window manager properties
  enum class icccm_wm_properties : u32_t {
    WM_CLASS,
    WM_CLIENT_MACHINE,
    WM_COLORMAP_WINDOWS,
    WM_HINTS,
    WM_ICON_NAME,
    WM_ICON_SIZE,
    WM_NAME,
    WM_NORMAL_HINTS,
    WM_PROTOCOLS,
    WM_STATE,
    WM_TRANSIENT_FOR,
    _icccm_wm_properties_end,
  };

  enum class icccm_session_properties : u32_t {
    WM_CLIENT_LEADER = (u32_t)icccm_wm_properties::_icccm_wm_properties_end,
    WM_COMMAND,
    SM_CLIENT_ID,
    _icccm_session_properties_end,
  };

  enum class icccm_wm_protocols : u32_t {
    WM_TAKE_FOCUS = (u32_t)icccm_session_properties::_icccm_session_properties_end,
    WM_SAVE_YOURSELF,
    WM_DELETE_WINDOW,
    _icccm_wm_protocols_end,
  };

  enum class icccm_wm_messages : u32_t {
    WM_CHANGE_STATE = (u32_t)icccm_wm_protocols::_icccm_wm_protocols_end,
    _icccm_wm_messages_end,
  };

  /// EWMH Root window messages
  enum class ewmh_root_window_messages : u32_t {
    _NET_CLOSE_WINDOW = (u32_t)icccm_wm_messages::_icccm_wm_messages_end,
    _NET_MOVERESIZE_WINDOW,
    _NET_WM_MOVERESIZE,
    _NET_RESTACK_WINDOW,
    _NET_REQUEST_FRAME_EXTENTS,
    _ewmh_root_window_mesasges_end
  };

  /// EWMH application window properties
  enum class ewmh_app_window_properties : u32_t {
    _NET_WM_NAME = (u32_t)ewmh_root_window_messages::_ewmh_root_window_mesasges_end,
    _NET_WM_VISIBLE_NAME,
    _NET_WM_ICON_NAME,
    _NET_WM_VISIBLE_ICON_NAME,
    _NET_WM_DESKTOP,
    _NET_WM_WINDOW_TYPE,
    _NET_WM_STATE,
    _NET_WM_ALLOWED_ACTIONS,
    _NET_WM_STRUT,
    _NET_WM_STRUT_PARTIAL,
    _NET_WM_ICON_GEOMETRY,
    _NET_WM_ICON,
    _NET_WM_PID,
    _NET_WM_HANDLED_ICONS,
    _NET_WM_USER_TIME,
    _NET_FRAME_EXTENTS,
    _ewmh_app_window_properties_end,
  };

  // EWMH window manager protocols
  enum class ewmh_wm_protocols : u32_t {
    _NET_WM_PING = (u32_t)ewmh_app_window_properties::_ewmh_app_window_properties_end,
    _NET_WM_SYNC_REQUEST,
    _ewmh_wm_protocols_end,
  };

  inline constexpr u32_t num_predefined_atoms = (u32_t)ewmh_wm_protocols::_ewmh_wm_protocols_end;
}
