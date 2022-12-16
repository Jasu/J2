#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/string.hpp"

namespace j::lisp::assembly::amd64 {
  J_RETURNS_NONNULL char * emit_nop(char * J_NOT_NULL ptr, u32_t len) noexcept
  {
    J_ASSUME(len > 0U);
    J_ASSUME(len < 10U);
    switch (len) {
    case 1: *ptr = 0x90; break;
    case 2: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x66, 0x90}, 2); break;
    case 3: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x0F, 0x1F, 0x00}, 3); break;
    case 4: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x0F, 0x1F, 0x40, 0x00}, 4); break;
    case 5: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x0F, 0x1F, 0x44, 0x00, 0x00}, 5); break;
    case 6: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00}, 6); break;
    case 7: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00}, 7); break;
    case 8: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00}, 8); break;
    case 9: J_MEMCPY_INLINE(ptr, (const u8_t*)(const u8_t[]){0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00}, 9); break;
    default: J_UNREACHABLE();
    }
    return ptr + len;
  }

  J_RETURNS_NONNULL char * emit_nops(char * J_NOT_NULL ptr, u32_t len) noexcept {
    J_ASSUME(len > 0U);
    J_ASSUME(len < 16U);
    if (len < 10) {
      return emit_nop(ptr, len);
    }
    switch (len) {
    case 10: return emit_nop(emit_nop(ptr, 3U), 7U);
    case 11: return emit_nop(emit_nop(ptr, 4U), 7U);
    case 12: return emit_nop(emit_nop(ptr, 4U), 8U);
    case 13: return emit_nop(emit_nop(ptr, 5U), 8U);
    case 14: return emit_nop(emit_nop(ptr, 7U), 7U);
    case 15: return emit_nop(emit_nop(ptr, 7U), 8U);
    default: J_UNREACHABLE();
    }
  }
}
