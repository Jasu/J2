#pragma once

#include "lisp/mem/heap_ctrl_value.hpp"
#include "lisp/values/lisp_object.hpp"
#include "lisp/values/lisp_str.hpp"
#include "lisp/values/lisp_vec.hpp"

namespace j::lisp::mem {
  struct heap_cell final {
    J_BOILERPLATE(heap_cell, CTOR_DEL, COPY_DEL, MOVE_DEL)

    object_header header;

    J_INLINE_GETTER bool is_object() const noexcept {
      return header.lowtag.is_object();
    }

    J_INLINE_GETTER bool is_ctrl() const noexcept {
      return header.lowtag.is_ctrl();
    }

    J_INLINE_GETTER lisp_object & as_object() noexcept {
      return reinterpret_cast<lisp_object &>(*this);
    }

    J_INLINE_GETTER heap_ctrl_value & as_ctrl() noexcept {
      return reinterpret_cast<heap_ctrl_value &>(*this);
    }

    J_INLINE_GETTER const lisp_object & as_object() const noexcept {
      return const_cast<heap_cell*>(this)->as_object();
    }

    J_INLINE_GETTER const heap_ctrl_value & as_ctrl() const noexcept {
      return const_cast<heap_cell*>(this)->as_ctrl();
    }

    J_INLINE_GETTER bool is_free_chunk() const noexcept {
      return header.lowtag.type() == lowtag_type::free_chunk;
    }

    J_INLINE_GETTER bool is_sentinel() const noexcept {
      return header.lowtag.type() == lowtag_type::sentinel;
    }

    J_INLINE_GETTER bool is_footer() const noexcept {
      return header.lowtag.type() == lowtag_type::footer;
    }

    J_INLINE_GETTER bool is_debug_info() const noexcept {
      return header.lowtag.type() == lowtag_type::debug_info;
    }

    J_INLINE_GETTER heap_free_chunk & as_free_chunk() noexcept {
      return as_ctrl().as_free_chunk();
    }

    J_INLINE_GETTER heap_sentinel & as_sentinel() noexcept {
      return as_ctrl().as_sentinel();
    }

    J_INLINE_GETTER heap_footer & as_footer() noexcept {
      return as_ctrl().as_footer();
    }

    J_INLINE_GETTER heap_debug_info & as_debug_info() noexcept {
      return as_ctrl().as_debug_info();
    }

    J_INLINE_GETTER const heap_free_chunk & as_free_chunk() const noexcept {
      return const_cast<heap_cell*>(this)->as_free_chunk();
    }

    J_INLINE_GETTER const heap_sentinel & as_sentinel() const noexcept {
      return const_cast<heap_cell*>(this)->as_sentinel();
    }

    J_INLINE_GETTER const heap_footer & as_footer() const noexcept {
      return const_cast<heap_cell*>(this)->as_footer();
    }

    J_INLINE_GETTER const heap_debug_info & as_debug_info() const noexcept {
      return const_cast<heap_cell*>(this)->as_debug_info();
    }

    [[nodiscard]] u32_t size_taken() const noexcept {
      if (is_object()) {
        return as_object().size_taken();
      } else {
        return as_ctrl().size_taken();
      }
    }

    J_INLINE_GETTER bool has_footer() const noexcept {
      return size_taken() > 8U;
    }

    [[nodiscard]] heap_footer & footer() noexcept {
      return (next() - 1)->as_footer();
    }

    [[nodiscard]] const heap_footer & footer() const noexcept {
      return const_cast<heap_cell*>(this)->footer();
    }

    // Impl in heap_ctrl_value
    [[nodiscard]] heap_cell * next() noexcept;

    J_INLINE_GETTER const heap_cell * next() const noexcept {
      return const_cast<heap_cell*>(this)->next();
    }

    J_INLINE_GETTER_NONNULL heap_cell * previous() noexcept {
      return this - 1;
    }

    J_INLINE_GETTER_NONNULL const heap_cell * previous() const noexcept {
      return this - 1;
    }
  };
}
