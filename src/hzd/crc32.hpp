#pragma once

#include "hzd/concepts.hpp"
#include "mem/memory_region.hpp"

namespace j {
  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, char value) noexcept {
    return __builtin_ia32_crc32qi(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, i8_t value) noexcept {
    return __builtin_ia32_crc32qi(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, i16_t value) noexcept {
    return __builtin_ia32_crc32hi(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, i32_t value) noexcept {
    return __builtin_ia32_crc32si(acc, value);
  }
  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, i64_t value) noexcept {
    return __builtin_ia32_crc32di(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, u8_t value) noexcept {
    return __builtin_ia32_crc32qi(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, u16_t value) noexcept {
    return __builtin_ia32_crc32hi(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, u32_t value) noexcept {
    return __builtin_ia32_crc32si(acc, value);
  }

  J_A(AI,ND,NODISC,FLATTEN) inline u32_t crc32(u32_t acc, u64_t value) noexcept {
    return __builtin_ia32_crc32di(acc, value);
  }

  [[nodiscard]] inline u32_t crc32(u32_t acc, const char * J_AA(NN) buffer, u32_t size) noexcept {
    if (size & 1) {
      acc = __builtin_ia32_crc32qi(acc, *(u8_t*)buffer);
      ++buffer;
    }
    if (size & 2) {
      acc = __builtin_ia32_crc32hi(acc, *(u16_t*)buffer);
      buffer += 2;
    }
    if (size & 4) {
      acc = __builtin_ia32_crc32si(acc, *(u32_t*)buffer);
      buffer += 4;
    }
    size >>= 3;
    const u64_t * buf = (const u64_t *)buffer;
    for (u32_t i = 0; i != size; ++i, ++buf) {
      acc = __builtin_ia32_crc32di(acc, *buf);
    }
    return acc;
  }

  [[nodiscard]] inline u32_t crc32(u32_t acc, const j::mem::const_memory_region & reg) noexcept
  { return crc32(acc, reg.begin(), reg.size()); }

  [[nodiscard]] inline u32_t crc32_64(
    u32_t acc,
    const void * J_NOT_NULL J_VAR_ALIGNED(8) const buffer,
    u32_t size
  ) noexcept {
    const u64_t * J_VAR_ALIGNED(8) buf = reinterpret_cast<const u64_t*>(buffer);
    while (size--) {
      acc = __builtin_ia32_crc32di(acc, *buf++);
    }
    return acc;
  }

  J_A(FLATTEN,NODISC) inline u32_t crc32_combine(u32_t crc32_lhs, u32_t crc32_rhs, u32_t rhs_size) noexcept {
    for (; rhs_size >= 8U; rhs_size -= 8U) {
      crc32_lhs = __builtin_ia32_crc32di(crc32_lhs, 0);
    }
    if (rhs_size & 4U) {
      crc32_lhs = __builtin_ia32_crc32si(crc32_lhs, 0);
    }
    if (rhs_size & 2U) {
      crc32_lhs = __builtin_ia32_crc32hi(crc32_lhs,  0);
    }
    if (rhs_size & 1U) {
      crc32_lhs = __builtin_ia32_crc32qi(crc32_lhs,  0);
    }
    return crc32_lhs ^ crc32_rhs;
  }
}
