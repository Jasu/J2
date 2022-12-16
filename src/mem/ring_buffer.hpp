#pragma once

#include "containers/pair.hpp"
#include "mem/memory_region.hpp"

namespace j::mem {
  using buffer_pair_t = pair<j::mem::memory_region>;

  /// Producer-consumer ring buffer storing bytes.
  ///
  /// The buffer is a power-of-two sized ring buffer, containing produces and
  /// consumer indices.
  ///
  /// *Logical* layout of the range:
  ///
  /// ```
  ///                WR index                     RD index
  ///                |                            |
  /// +--------------|----------------------------|--------------+
  /// | Readable     | Writable                   | Readable     |
  /// +----------------------------------------------------------+
  ///
  /// The Initial layout, where only writable (clean) memory exists:
  ///
  /// rd_iter                                                    wr_iter
  /// wr_iter                                                    rd_iter
  /// |                                                          |
  /// +----------------------------------------------------------+
  /// |                    Writable                              |
  /// +----------------------------------------------------------+
  /// ```
  ///
  /// Note that RD and WR are both at the same position, i.e. the
  /// wrap point of the buffer. Yet a writable range exist, while
  /// readable doesn't. To represent this, both indices are stored
  /// as `u32_t` that are never wrapped (except by unsigned overflow).
  ///
  /// Then:
  ///   - Index can be computed by `iter & (capacity - 1)`
  ///   - Writable length: `rd_iter - wr_iter`
  ///   - Readable length: `capacity - rd_iter + wr_iter`
  ///   - Is write region split: `(rd_iter ^ wr_iter) & capacity`
  ///   - Is read region split: `!((rd_iter ^ wr_iter) & capacity)`

  struct ring_buffer final {
    J_BOILERPLATE(ring_buffer, CTOR_DEL, COPY_DEL, MOVE_DEL)

    char * base = nullptr;
    u32_t it_wr = 0U;
    u32_t it_rd = 0U;
    u32_t capacity = 0U;

    explicit ring_buffer(u32_t capacity);

    ~ring_buffer();

    u32_t write(const j::mem::const_memory_region & buffer) noexcept {
      return write(buffer.begin(), buffer.size());
    }

    /// Writes data to the ring buffer.
    u32_t write(const char * J_NOT_NULL from, u32_t size) noexcept;
    u32_t read(char * J_NOT_NULL to, u32_t size) noexcept;

    j::mem::memory_region get_write_buffer(u32_t size) noexcept;
    j::mem::memory_region get_read_buffer(u32_t size) noexcept;
    buffer_pair_t get_write_buffers(u32_t size) noexcept;
    buffer_pair_t get_read_buffers(u32_t size) noexcept;

    J_INLINE_GETTER u32_t writable_size() const noexcept {
      return it_rd - it_wr;
    }

    J_INLINE_GETTER u32_t readable_size() const noexcept {
      return capacity - it_rd + it_wr;
    }

    J_INLINE_GETTER u32_t writable_index() const noexcept {
      return it_wr & (capacity - 1U);
    }

    J_INLINE_GETTER u32_t readable_index() const noexcept {
      return it_rd & (capacity - 1U);
    }

    J_ALWAYS_INLINE void mark_as_read(u32_t size) noexcept {
      it_rd += size;
    }
    J_ALWAYS_INLINE void mark_as_written(u32_t size) noexcept {
      it_rd += size;
    }

    void dump(const char * label = nullptr) const noexcept;
  };
}
