#pragma once

#include "lisp/cir/liveness/segment.hpp"
#include "hzd/mem.hpp"

namespace j::lisp::cir::inline ops {
  struct op;
}
namespace j::lisp::cir::inline liveness {
  template<typename, typename> struct basic_segment_iterator;

  struct live_range final {
    using iterator = basic_segment_iterator<segment, live_range>;
    using const_iterator = basic_segment_iterator<const segment, const live_range>;

    J_BOILERPLATE(live_range, CTOR_NE_ND, MOVE_DEL)

    [[nodiscard]] iterator begin() noexcept;
    [[nodiscard]] const_iterator begin() const noexcept;
    [[nodiscard]] iterator end() noexcept;
    [[nodiscard]] const_iterator end() const noexcept;

    [[nodiscard]] iterator front() noexcept;
    [[nodiscard]] const_iterator front() const noexcept;
    [[nodiscard]] iterator back() noexcept;
    [[nodiscard]] const_iterator back() const noexcept;

    J_INLINE_GETTER bool empty() const noexcept { return !size; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return size; }
    J_INLINE_GETTER bool operator!() const noexcept { return !size; }

    iterator split_at(op_index i, loc to) noexcept;

    iterator move_to_loc_at(op_index i, loc to) noexcept;

    inline constexpr void clear() noexcept {
      if (size) {
        j::free(m_data.ptr);
        size = 0U;
      }
    }

    J_A(AI) inline ~live_range() { clear(); }

    void add(segment h);

    [[nodiscard]] bool contains(op_index i) const noexcept;

    [[nodiscard]] iterator segment_at(op_index i) noexcept;

    [[nodiscard]] const_iterator segment_at(op_index i) const noexcept;

    [[nodiscard]] iterator segment_at_or_after(op_index i) noexcept;

    [[nodiscard]] const_iterator segment_at_or_after(op_index i) const noexcept;

    [[nodiscard]] op_index lower_bound() const noexcept;

    [[nodiscard]] op * source_op() const noexcept;

    [[nodiscard]] op_index upper_bound() const noexcept {
      switch (size) {
      case 0: return op_index();
      case 1: return m_data.val.index_end;
      default: return m_data.ptr[size - 1].index_end;
      }
    }

    u16_t size = 0U;
  private:
    union {
      segment val;
      segment * ptr;
    } m_data = {{}};
  };
}
