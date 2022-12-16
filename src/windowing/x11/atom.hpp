#pragma once

#include "containers/unsorted_string_map_fwd.hpp"
#include "windowing/x11/default_atoms.hpp"

extern "C" {
  typedef u32_t xcb_atom_t;
  typedef struct xcb_connection_t xcb_connection_t;
}

namespace j::windowing::x11 {
  namespace detail {
    xcb_atom_t get_atom(xcb_connection_t * c, u32_t index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, icccm_wm_properties index) {
    return detail::get_atom(c, (u32_t)index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, icccm_session_properties index) {
    return detail::get_atom(c, (u32_t)index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, icccm_wm_protocols index) {
    return detail::get_atom(c, (u32_t)index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, icccm_wm_messages index) {
    return detail::get_atom(c, (u32_t)index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, ewmh_root_window_messages index) {
    return detail::get_atom(c, (u32_t)index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, ewmh_app_window_properties index) {
    return detail::get_atom(c, (u32_t)index);
  }
  J_INLINE_GETTER xcb_atom_t get_atom(xcb_connection_t * c, ewmh_wm_protocols index) {
    return detail::get_atom(c, (u32_t)index);
  }
}
