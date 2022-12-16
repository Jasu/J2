#include "mem/ring_buffer.hpp"
#include "logging/global.hpp"

namespace j::mem {

  ring_buffer::ring_buffer(u32_t capacity)
    : base((char*)j::allocate_aligned(64, capacity)),
      it_wr(0),
      it_rd(capacity),
      capacity(capacity)
  {
  }

  ring_buffer::~ring_buffer() {
    if (base) {
      j::free(base);
    }
  }

  j::mem::memory_region ring_buffer::get_write_buffer(u32_t size) noexcept {
    J_ASSUME(size);
    u32_t wr_index = writable_index();
    size = min(size, writable_size(), capacity - wr_index);
    it_wr += size;
    return {base + wr_index, size};
  }

  j::mem::memory_region ring_buffer::get_read_buffer(u32_t size) noexcept {
    J_ASSUME(size);
    u32_t rd_index = readable_index();
    size = min(size, readable_size(), capacity - rd_index);
    it_rd += size;
    return {base + rd_index, size};
  }

  buffer_pair_t ring_buffer::get_write_buffers(u32_t size) noexcept {
    J_ASSUME(size);
    size = min(size, writable_size());
    u32_t wr_index = writable_index();
    it_wr += size;
    u32_t wr_sz = min(size, capacity - wr_index);
    return {
      {base + wr_index, wr_sz},
      {base, size - wr_sz}
    };
  }

  buffer_pair_t ring_buffer::get_read_buffers(u32_t size) noexcept {
    J_ASSUME(size);
    size = min(size, readable_size());
    u32_t rd_index = readable_index();
    it_rd += size;
    u32_t rd_sz = min(size, capacity - rd_index);
    return {
      {base + rd_index, rd_sz},
      {base, size - rd_sz}
    };
  }

  u32_t ring_buffer::write(const char * J_NOT_NULL from, u32_t size) noexcept {
    J_ASSUME(size);
    size = min(size, writable_size());
    if (!size) {
      return 0U;
    }
    u32_t wr_index = writable_index();
    it_wr += size;
    u32_t wr_sz = min(size, capacity - wr_index);
    ::j::memcpy(base + wr_index, from, wr_sz);
    if (size != wr_sz) {
      ::j::memcpy(base, from + wr_sz, size - wr_sz);
    }
    return size;
  }

  u32_t ring_buffer::read(char * J_NOT_NULL to, u32_t size) noexcept {
    J_ASSUME(size);
    size = min(size, readable_size());
    u32_t rd_index = readable_index();
    it_rd += size;
    u32_t rd_sz = min(size, capacity - rd_index);
    ::j::memcpy(to, base + rd_index, rd_sz);
    if (size != rd_sz) {
      ::j::memcpy(to + rd_sz, base, size - rd_sz);
    }
    return size;
  }

  void ring_buffer::dump(const char * label) const noexcept {
    label = label ? label : "Ring buffer";
    J_DEBUG("{#bright_cyan_bg,white,bold} {} {/} Writable: {}b Readable: {}b", label, writable_size(), readable_size());
    i32_t max_x = 70;
    J_DEBUG(" 0{indent}{:7}", 0, max_x - 1 - 7, capacity);
    const char * const wr_line = "{#bright_red,bold}┅{/}";
    const char * const rd_line = "{#bright_green,bold}┅{/}";
    const char * cur_line = rd_line;
    const i32_t wr = writable_index(), rd = readable_index();
    strings::string str = " ";
    for (u8_t i = 0; i < max_x; ++i) {
      const int begin_pos = (capacity * i) / max_x;
      const int end_pos = (capacity * (i + 1)) / max_x;
      const bool at_rd = rd >= begin_pos && rd < end_pos;
      const bool at_wr = wr >= begin_pos && wr < end_pos;
      if (at_rd && at_wr) {
        str += "{#bright_yellow,bold}X{/}";
      } else if (at_rd) {
        str += "{#bright_green,bold}R{/}";
        cur_line = rd_line;
      } else if (wr >= begin_pos && rd < end_pos) {
        str += "{#bright_red,bold}W{/}";
        cur_line = wr_line;
      } else {
        str += cur_line;
      }
    }
    J_DEBUG(str);
  }
}
