#pragma once

#include "logging/global.hpp"
#include "hzd/string.hpp"
#include "hzd/mem.hpp"
#include "os/dwarf_common.hpp"
#include "exceptions/assert.hpp"

namespace j::os::dwarf {
  struct base_reader {
    const char * offset = nullptr;
    const char * end = nullptr;

    J_A(AI,NODISC,ND) inline explicit operator bool() const noexcept {
      return offset != end;
    }

    J_A(AI,NODISC,ND) inline bool operator!() const noexcept {
      return offset == end;
    }

    J_A(AI,HIDDEN,ND) inline base_reader() noexcept = default;

    J_A(HIDDEN,ND) inline base_reader(const char * J_AA(NN) begin, const char * J_AA(NN) end) noexcept
    : offset(begin),
      end(end)
      {
        J_ASSERT(begin <= end);
      }

    J_A(HIDDEN) inline void check_offset(i32_t size) const {
      J_ASSERT(offset && end && size > 0 && end - offset >= size, "Unexpected end of section (need {} bytes, have {})", size, end - offset);
    }

    J_A(RNN,HIDDEN) inline const char * consume(i32_t size) {
      const char * it = offset;
      offset += size;
      J_ASSERT(size >= 0 && offset <= end, "Expected {} bytes, have {}", size, end - offset + size);
      return it;
    }

#define J_DEF_READERS(T)                                          \
    J_A(AI,NODISC) inline T peek_##T() const {                    \
      check_offset(sizeof(T));                                    \
      return *(const T*)offset;                                   \
    }                                                             \
    J_A(AI,NODISC) inline T peek_##T##_unsafe() const noexcept {  \
      return *(const T*)offset;                                   \
    }                                                             \
    J_A(AI,NODISC) inline T read_##T() {                          \
      return *(const T*)consume(sizeof(T));                       \
    }                                                             \
    J_A(AI,NODISC) inline T read_##T##unsafe() noexcept {         \
      T result = *(const T*)offset;                               \
      offset += sizeof(T);                                        \
      return result;                                              \
    }

    J_DEF_READERS(sbyte)
    J_DEF_READERS(ubyte)
    J_DEF_READERS(uhalf)
    J_DEF_READERS(shalf)
    J_DEF_READERS(uword)
    J_DEF_READERS(sword)
    J_DEF_READERS(udword)
    J_DEF_READERS(sdword)

    J_A(AI,NODISC) inline uword peek_uthreequarters() const {
      check_offset(3);
      return {peek_uword_unsafe() & 0x00FFFFFFU};
    }
    J_A(AI,NODISC) inline uword peek_uthreequarters_unsafe() const noexcept {
      return {peek_uword_unsafe() & 0x00FFFFFFU};
    }
    J_A(AI,NODISC) inline uword read_uthreequarters() {
      return {*(u32_t*)consume(3) & 0x00FFFFFFU};
    }
    J_A(AI,NODISC) inline uword read_uthreequarters_unsafe() noexcept {
      u32_t * result = (u32_t*)offset;
      offset += 3;
      return {*result & 0x00FFFFFFU};
    }

    template<typename T>
    J_A(AI,ND,HIDDEN) inline void read_to(T & target) {
      j::memcpy(&target, consume(sizeof(T)), sizeof(T));
    }

    template<typename T>
    J_A(AI,ND,HIDDEN,RNN,NODISC) inline const T* read_ptr() {
      return (const T*)consume(sizeof(T));
    }

    template<typename T>
    J_A(AI,ND,HIDDEN,RNN,NODISC) inline const T* peek_ptr() const {
      check_offset(sizeof(T));
      return (const T*)offset;
    }

    template<typename T>
    J_A(AI,ND,HIDDEN) inline void peek_to(T & target) {
      check_offset(sizeof(T));
      j::memcpy(&target, offset, sizeof(T));
    }

    J_A(AI) inline void skip_leb128() {
      while (read_ubyte() & 0x80) { }
    }

    J_A(RNN) inline const char* read_str() {
      const char* result = offset;
      offset = j::memchr(offset, '\0', end - offset);
      J_REQUIRE(offset, "Unterminated string");
      return result;
    }

    J_A(AI,ND) inline void skip_zeroes() noexcept {
      while (offset != end && !*offset) {
        ++offset;
      }
    }

    J_A(NODISC) inline u64_t read_abbrev_code() {
      if (offset != end) {
        return read_uleb128();
      }
      return 0;
    }

    J_A(NODISC,AI,HIDDEN,ND) inline u64_t read_leb128(u8_t & J_AA(NOALIAS) shift, u8_t & J_AA(NOALIAS) byte) {
      u64_t result = 0;
      do {
        byte = read_ubyte();
        result |= (u64_t)(byte & 0x7F) << shift;
        shift += 7;
      } while (byte & 0x80);
      return result;
    }

    J_A(NODISC) u64_t read_uleb128() {
      u8_t shift = 0, byte;
      return read_leb128(shift, byte);
    }


    J_A(NODISC) i64_t read_sleb128() {
      u8_t shift = 0, byte;
      i64_t result = read_leb128(shift, byte);
      if (shift < 64 && byte & 0x40) {
        return result - (1L << (shift - 1));
      }
      return result;
    }

    J_A(AI,NODISC,ND) inline bool is_at_end() const noexcept {
      return offset == end;
    }

    J_A(AI,NODISC,ND) inline base_reader read_with_subreader(i32_t size) {
      const char * sub_begin = consume(size);
      return {sub_begin, offset};
    }

    J_A(NODISC,AI,HIDDEN) inline bool is_dwarf64() const {
      return peek_sword() == -1;
    }
  };
}
