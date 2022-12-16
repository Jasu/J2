#pragma once

#include "lisp/cir/ssa/bb_iterator.hpp"
#include "lisp/cir/ssa/bb_op_iterator.hpp"

namespace j::lisp::cir::inline ops {
  struct op;
}

namespace j::lisp::cir::inline ssa {
  template<typename Bb, typename Op, bool IsReversed>
  struct basic_ssa_op_iterator final {
    J_BOILERPLATE(basic_ssa_op_iterator, EQ)

    basic_bb_iterator<Bb, IsReversed> bb_it, bb_end;
    bb_op_iterator<Op, IsReversed> op_it;

    J_A(ND) inline basic_ssa_op_iterator(bb_iterator_view<Bb, IsReversed> view, bool is_end) noexcept
      : bb_it(is_end ? view.end() : view.begin()),
        bb_end(view.end())
    {
      if (bb_it != bb_end) {
        if constexpr (IsReversed) {
          op_it = bb_it->rbegin();
        } else {
          op_it = bb_it->begin();
        }
      }
    }

    J_INLINE_GETTER_NONNULL auto * operator->() const noexcept {
      return op_it.operator->();
    }

    J_INLINE_GETTER auto & operator*() const noexcept {
      return op_it.operator*();
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !op_it;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return op_it;
    }

    basic_ssa_op_iterator & operator++() noexcept {
      if (!++op_it) {
        if (++bb_it != bb_end) {
          if constexpr (IsReversed) {
            op_it = bb_it->rbegin();
          } else {
            op_it = bb_it->begin();
          }
        }
      }
      return *this;
    }

    basic_ssa_op_iterator & operator++(int) noexcept {
      basic_ssa_op_iterator result{*this};
      operator++();
      return result;
    }
  };

  template<bool IsReversed>
  basic_ssa_op_iterator(bb_iterator_view<bb, IsReversed>, bool) -> basic_ssa_op_iterator<bb, op, IsReversed>;
  template<bool IsReversed>
  basic_ssa_op_iterator(bb_iterator_view<const bb, IsReversed>, bool) -> basic_ssa_op_iterator<const bb, const op, IsReversed>;
}
