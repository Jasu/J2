#pragma once

#include "lisp/mem/object_header.hpp"
#include "lisp/mem/debug_info.hpp"
#include "lisp/sources/source_location.hpp"
#include "hzd/mem.hpp"

namespace j::lisp::mem {
  struct heap_cell;

  struct heap_sentinel;
  struct heap_footer;
  struct heap_free_chunk;
  struct heap_debug_info;

  struct heap_ctrl_value {
    object_header header;

  protected:
    J_ALWAYS_INLINE constexpr heap_ctrl_value() noexcept = default;

    template<typename... Args>
    J_ALWAYS_INLINE constexpr heap_ctrl_value(Args && ... args) noexcept
      : header(static_cast<Args &&>(args)...)
    { }

    template<typename T>
    J_INLINE_GETTER_NONNULL T * data() noexcept {
      return reinterpret_cast<T*>(this + 1);
    }

    template<typename T>
    J_INLINE_GETTER_NONNULL const T * data() const noexcept {
      return reinterpret_cast<const T*>(this + 1);
    }
  public:
    J_INLINE_GETTER constexpr lowtag_type type() const noexcept {
      return header.lowtag.type();
    }

    J_INLINE_GETTER constexpr bool is_free_chunk() const noexcept {
      return header.lowtag.type() == lowtag_type::free_chunk;
    }

    J_INLINE_GETTER constexpr bool is_sentinel() const noexcept {
      return header.lowtag.type() == lowtag_type::sentinel;
    }

    J_INLINE_GETTER constexpr bool is_footer() const noexcept {
      return header.lowtag.type() == lowtag_type::footer;
    }

    J_INLINE_GETTER constexpr bool is_debug_info() const noexcept {
      return header.lowtag.type() == lowtag_type::debug_info;
    }

    J_INLINE_GETTER heap_free_chunk & as_free_chunk() noexcept {
      return reinterpret_cast<heap_free_chunk &>(*this);
    }

    J_INLINE_GETTER heap_sentinel & as_sentinel() noexcept {
      return reinterpret_cast<heap_sentinel&>(*this);
    }

    J_INLINE_GETTER heap_footer & as_footer() noexcept {
      return reinterpret_cast<heap_footer&>(*this);
    }

    J_INLINE_GETTER heap_debug_info & as_debug_info() noexcept {
      return reinterpret_cast<heap_debug_info&>(*this);
    }

    J_INLINE_GETTER const heap_free_chunk & as_free_chunk() const noexcept {
      return const_cast<heap_ctrl_value*>(this)->as_free_chunk();
    }

    J_INLINE_GETTER const heap_sentinel & as_sentinel() const noexcept {
      return const_cast<heap_ctrl_value*>(this)->as_sentinel();
    }

    J_INLINE_GETTER const heap_footer & as_footer() const noexcept {
      return const_cast<heap_ctrl_value*>(this)->as_footer();
    }

    J_INLINE_GETTER const heap_debug_info & as_debug_info() const noexcept {
      return const_cast<heap_ctrl_value*>(this)->as_debug_info();
    }

    [[nodiscard]] u32_t size_taken() const noexcept;

    [[nodiscard]] J_RETURNS_NONNULL heap_ctrl_value * next() noexcept {
      return add_bytes(this, size_taken());
    }

    J_INLINE_GETTER_NONNULL const heap_ctrl_value * next() const noexcept {
      return const_cast<heap_ctrl_value *>(this)->next();
    }
  };

  struct heap_sentinel final : heap_ctrl_value {
    J_ALWAYS_INLINE constexpr heap_sentinel(bool is_end) noexcept
      : heap_ctrl_value(sentinel_tag, is_end)
    { }

    J_INLINE_GETTER bool is_end() const noexcept {
      return header.lowtag.data();
    }

    J_INLINE_GETTER bool is_begin() const noexcept {
      return !is_end();
    }
  };

  struct heap_footer final : heap_ctrl_value {
    J_ALWAYS_INLINE heap_footer(const heap_cell * J_NOT_NULL start, bool is_free_chunk) noexcept
      : heap_ctrl_value(footer_tag, byte_offset(start, this) >> 3U, is_free_chunk)
    { }

    J_INLINE_GETTER u32_t offset_qwords() const noexcept {
      return header.lowtag.data();
    }

    J_INLINE_GETTER_NONNULL heap_cell * start() const noexcept {
      return subtract_bytes<heap_cell*>(this, header.lowtag.data() << 3U);
    }

    J_ALWAYS_INLINE void set_start(const heap_cell * J_NOT_NULL start) noexcept {
      header.lowtag.set_data(byte_offset(start, this) >> 3U);
    }
  };

  struct heap_free_chunk final : heap_ctrl_value {
    J_ALWAYS_INLINE heap_free_chunk(u32_t size_qwords, heap_free_chunk * next) noexcept
      : heap_ctrl_value(free_chunk_tag, size_qwords, next ? next - this : 0U)
    {
    }

    J_INLINE_GETTER u32_t size_qwords() const noexcept {
      return header.lowtag.data();
    }

    J_INLINE_GETTER heap_free_chunk * next_free() const noexcept {
      u32_t offset = header.free_chunk_hightag().next_free_offset;
      return const_cast<heap_free_chunk*>(offset ? this + offset : nullptr);
    }

    J_ALWAYS_INLINE void set_next_free(heap_free_chunk * next) noexcept {
      header.free_chunk_hightag().next_free_offset = next ? next - this : 0U;
    }
  };

  struct heap_debug_info final : heap_ctrl_value {
    heap_debug_info(debug_info_type type) noexcept;

    heap_debug_info(detail::source_location_tag_t, const sources::source_location & loc) noexcept;

    J_INLINE_GETTER debug_info_type debug_info_type() const noexcept {
      return (enum debug_info_type)header.lowtag.data();
    }

    [[nodiscard]] sources::source_location & source_location() noexcept;

    J_INLINE_GETTER const sources::source_location & source_location() const noexcept {
      return const_cast<heap_debug_info *>(this)->source_location();
    }
  };
}
