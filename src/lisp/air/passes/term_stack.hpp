#pragma once

///   Term stack   Stack of each nonterminal's
///   +--------+   stack pointers        +-------+
/// 0 | expr*  |                         | Progn |
///   +--------+     +--------+          +-------+------------------+
/// 1 | expr*  |<----| expr** |<---------| expr*** stack allocation |
///   +--------+     +--------+          +--------------------------+
/// 2 | expr*  |  +--| expr** |<----+
///   +--------+  |  +--------+     |    +-----------+
/// 3 | expr*  |<-+                 |    | VecAppend |
///   +--------+                    |    +-----------+--------------+
///                                 +----| expr*** stack_allocation |
///                                      +--------------------------+
///
/// Here, `Progn` holds items 0 and 1 while `VecAppend` holds 2 and 3.

#include "logging/global.hpp"
#include "meta/attr_value.hpp"
#include "mem/bump_pool.hpp"

namespace j::lisp::air::exprs {
  struct expr;
}

namespace j::lisp::air::inline passes {
  namespace {
    using exprs::expr;
    struct J_TYPE_HIDDEN term_stack;

    template<i32_t NumMarkers>
    struct J_TYPE_HIDDEN tsa_base {
      term_stack * stack = nullptr;
      expr ***marker = nullptr;

      void push(expr * J_NOT_NULL e) noexcept {
        check();
        *(*marker)++ = e;
      }

      void push(span<expr*> es) noexcept {
        check();
        i32_t sz = es.size();
        j::memmove(**marker, es.begin(), sz * J_PTR_SZ);
        *marker += sz;
      }

      J_A(NODISC) inline operator span<expr*>() const noexcept {
        return check(false), marker ? span(marker[-NumMarkers], marker[0]) : span<expr*>();
      }


      J_A(NODISC) inline expr* & operator[](i32_t idx) const noexcept {
        return check(true, true), marker[-NumMarkers][idx];
      }

      J_A(NODISC) inline expr ** begin() const noexcept {
        return marker ? marker[-NumMarkers] : nullptr;
      }

      J_A(NODISC) inline expr ** end() const noexcept {
        return marker ? marker[0] : nullptr;
      }

      J_A(NODISC) inline expr * & front() noexcept {
        return check(true, true), *marker[-NumMarkers];
      }

      J_A(NODISC) inline expr * & back() noexcept {
        return check(true, true), marker[0][-1];
      }

      J_A(NODISC) inline i32_t size() const noexcept {
        return check(false), marker ? marker[0] - marker[-NumMarkers] : 0;
      }

      J_A(NODISC) inline explicit operator bool() const noexcept {
        return check(false), marker && marker[0] != marker[-NumMarkers];
      }

      J_A(NODISC) inline bool operator!() const noexcept {
        return check(false), !marker || marker[0] == marker[-NumMarkers];
      }

      J_A(AI,HIDDEN) inline span<expr*> release() noexcept;

      J_A(NI) inline void check(bool allocated = true, bool not_empty = false) const noexcept {
        if (allocated || marker) {
          J_ASSERT_NOT_NULL(marker, marker[-NumMarkers], marker[0], stack);
          J_ASSERT(not_empty ? marker[-NumMarkers] < marker[0] : marker[-NumMarkers] <= marker[0]);
        }
      }

      J_A(NODISC,AI,HIDDEN,ND) inline bool is_allocated() const noexcept {
        return marker;
      }
    };

    struct J_TYPE_HIDDEN term_stack final {
      expr ***markers;
      expr **data_begin;
      expr **data_end;
      expr ***markers_begin;
      expr ***markers_end;


      term_stack(j::mem::bump_pool & pool, i32_t num_exprs, i32_t num_slots) noexcept
        : markers((expr***)pool.allocate_aligned_zero(J_PTR_SZ * num_slots, J_PTR_SZ)),
          data_begin((expr**)pool.allocate_aligned_zero(J_PTR_SZ * num_exprs, J_PTR_SZ)),
          data_end(data_begin + num_exprs),
          markers_begin(markers),
          markers_end(markers + num_slots)
      {
        J_ASSERT(num_exprs && num_slots && num_slots < num_exprs);
        *markers = data_begin;
      }

      void erase(span<expr*> span) {
        expr ** b = span.begin(), **e = span.end();
        J_ASSERT(b >= data_begin && e <= *markers);
        j::memmove(b, e, (*markers - e) * J_PTR_SZ);
        for (expr *** m = markers_begin, ***max = markers + 1; m != max; ++m) {
          if (*m < b) {
            continue;
          }
          if (*m > e) {
            *m -= e - b;
          } else {
            *m = b;
          }
        }
      }

      void splice(expr *** J_NOT_NULL begin, span<expr*> span, bool move_marker_after) {
        if (!span) {
          return;
        }
        J_ASSERT(begin <= markers && begin >= markers_begin);
        J_ASSERT(span.end() <= data_begin || span.begin() >= data_end);
        expr ** end = *begin + span.size();
        J_ASSERT(*begin >= data_begin && end <= data_end && end > *begin);
        expr ** all_data_end = *markers;
        J_ASSERT(all_data_end <= data_end && all_data_end + span.size() <= data_end);
        j::memmove(end, *begin, (all_data_end - end) * J_PTR_SZ);
        j::memcpy(*begin, span.begin(), span.size() * J_PTR_SZ);
        for (expr *** m = begin + move_marker_after, ***max = markers + 1; m != max; ++m) {
          *m += span.size();
        }
      }

      J_A(RNN) inline expr *** allocate_marker() noexcept {
        J_ASSERT_NOT_NULL(markers, *markers);
        J_ASSERT(markers >= markers_begin && markers < markers_end - 1);
        *++markers = *markers;
        return markers;
      }

      J_A(RNN) inline expr *** allocate_marker(i32_t offset) noexcept {
        J_ASSERT(offset >= 0);
        J_ASSERT_NOT_NULL(markers, *markers);
        J_ASSERT(markers >= markers_begin && markers < markers_end - 1);
        J_ASSERT(*markers + offset <= data_end);
        *++markers = *markers + offset;
        return markers;
      }


      J_A(NODISC,RNN) inline expr *** merge(expr *** J_NOT_NULL lhs, expr *** J_NOT_NULL rhs, i32_t num_markers) noexcept {
        J_ASSERT(&lhs != &rhs && markers - markers_begin >= 2 * num_markers);
        J_ASSERT(lhs == markers || lhs == markers - num_markers);
        J_ASSERT(rhs == markers || rhs == markers - num_markers);
        J_ASSERT_NOT_NULL(*lhs, *rhs, lhs[-num_markers], rhs[-num_markers]);
        J_ASSERT(lhs[-num_markers] <= *lhs);
        J_ASSERT(rhs[-num_markers] <= *rhs);

        markers[-num_markers] = markers[0];
        while (num_markers--) {
          *markers-- = nullptr;
        }
        return markers;
      }

      inline span<expr*> release_markers(expr *** J_NOT_NULL end, i32_t count) noexcept {
        J_ASSERT(count);
        expr *** begin = end - count;
        J_ASSERT(end == markers);
        span<expr*> result{*begin, *end};
        while (count--) {
          *markers-- = nullptr;
        }
        return result;
      }

      J_A(NI) inline void check_top(expr *** marker) {
        J_ASSERT_NOT_NULL(marker, *marker, markers[0], markers[-1]);
        J_ASSERT(marker == markers, "Marker is not the topmost marker. Marker at #{} released, stack has {} markers.");
        J_ASSERT(markers[0] >= markers[-1]);
      }

      J_A(AI,RNN,NODISC,ND) inline expr *** begin() const noexcept {
        J_ASSERT(markers_begin && markers && markers_begin <= markers);
        return markers_begin;
      }

      J_A(AI,RNN,NODISC,ND) inline expr *** end() const noexcept {
        J_ASSERT(markers_begin && markers && markers_begin <= markers);
        return markers;
      }

      J_A(AI,NODISC,ND) inline bool empty() const noexcept {
        return data_begin == *markers;
      }
    };

    enum J_A(FLAGS) term_stack_type {
      stk_none    = 0b00,
      stk_collect = 0b01,
      stk_side_fx = 0b10,
      stk_all     = 0b11,
    };

    struct J_TYPE_HIDDEN term_stack_allocation final : tsa_base<1> {
      J_BOILERPLATE(term_stack_allocation, CTOR_NE)

      J_A(AI,HIDDEN) inline term_stack_allocation(term_stack * J_NOT_NULL stack, expr ***marker) noexcept
        : tsa_base{stack, marker}
      { }

      inline span<expr*> clear() noexcept {
        if (!marker) {
          return {};
        }
        check();
        u32_t sz = marker[0] - marker[-1];
        marker[0] = marker[-1];
        return span(marker[0], sz);
      }

      J_A(AI,HIDDEN) inline void reset() noexcept {
        marker = nullptr;
        stack = nullptr;
      }

      [[nodiscard]] inline term_stack_allocation operator+(term_stack_allocation & from) noexcept {
        J_ASSERT(stack && stack == from.stack);
        expr *** end = stack->merge(marker, from.marker, 1);
        return term_stack_allocation(stack, end);
      }

      inline term_stack_allocation & operator+=(term_stack_allocation & from) noexcept {
        J_ASSERT(stack && stack == from.stack);
        marker = stack->merge(marker, from.marker, 1);
        from.marker = nullptr;
        return *this;
      }

      template<typename T>
      term_stack_allocation & push(T && e) noexcept {
        tsa_base::push(static_cast<T &&>(e));
        return *this;
      }
    };

    struct J_TYPE_HIDDEN tsa_section final {
      expr *** begin;
      expr *** end;

      J_A(AI,ND,NODISC) inline i32_t num_markers() const noexcept {
        return end - begin;
      }

      J_A(AI,ND,NODISC) inline operator span<expr*>() const noexcept {
        return span(*begin, *end);
      }
      J_A(AI,ND,NODISC) inline expr* & operator[](i32_t index) const noexcept {
        return *begin[index];
      }
      J_A(AI,ND,NODISC) inline expr* & front() const noexcept {
        return **begin;
      }
      J_A(AI,ND,NODISC) inline expr* & back() const noexcept {
        return (*end)[-1];
      }
      J_A(AI,ND,NODISC) inline i32_t size() const noexcept { return *end - *begin; }
      J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return *begin != *end; }
      J_A(AI,ND,NODISC) inline bool operator!() const noexcept { return *begin == *end; }
    };

    struct J_TYPE_HIDDEN split_span final {
      expr **p0 = nullptr, **p1 = nullptr, **p2 = nullptr;

      J_A(NODISC,AI) inline operator bool() const noexcept {
        return p2 != p0;
      }
      J_A(NODISC,AI) inline bool operator!() const noexcept {
        return p2 == p0;
      }
      J_A(NODISC,AI) inline bool empty() const noexcept {
        return p2 == p0;
      }
      J_A(NODISC,AI,RNN) inline expr * & operator[](i32_t i) const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2 && p0 + i < p2);
        return p0[i];
      }

      J_A(NODISC,AI) inline i32_t size() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return p2 - p0;
      }
      J_A(NODISC,AI) inline i32_t num_pre_fx() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return p1 - p0;
      }
      J_A(NODISC,AI) inline i32_t num_post_fx() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return p2 - p1;
      }

      J_A(NODISC,AI,RNN) inline expr ** begin() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return p0;
      }
      J_A(NODISC,AI,RNN) inline expr ** end() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return p2;
      }

      J_A(NODISC,AI) inline operator span<expr*>() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return {p0, p2};
      }
      J_A(NODISC,AI) inline span<expr*> pre_fx() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return {p0, p1};
      }
      J_A(NODISC,AI) inline span<expr*> post_fx() const noexcept {
        J_ASSERT(p0 && p0 <= p1 && p1 <= p2);
        return {p0, p1};
      }
      J_A(AI) inline split_span & to_pre_fx() noexcept {
        p1 = p2;
        return *this;
      }
      J_A(AI) inline split_span & to_post_fx() noexcept {
        p1 = p0;
        return *this;
      }
    };

    struct J_TYPE_HIDDEN split_tsa final : tsa_base<2> {
      J_BOILERPLATE(split_tsa, CTOR_NE)

      J_A(AI,HIDDEN) inline split_tsa(term_stack * J_NOT_NULL stack, expr *** J_NOT_NULL end) noexcept
        : tsa_base{stack, end}
      { }

      inline span<expr*> clear() noexcept {
        if (!marker) {
          return {};
        }
        check();
        u32_t sz = marker[0] - marker[-2];
        return span(marker[0], sz);
      }

      J_A(AI,HIDDEN) inline void reset() noexcept {
        marker = nullptr;
        stack = nullptr;
      }

      inline split_tsa & to_pre_fx() noexcept {
        J_ASSERT_NOT_NULL(marker, *marker, marker[-1], marker[-2]);
        J_ASSERT(marker[-1] >= marker[-2] && marker[-1] <= *marker);
        marker[-1] = marker[0];
        return *this;
      }

      inline split_tsa & to_post_fx() noexcept {
        J_ASSERT_NOT_NULL(marker, *marker, marker[-1], marker[-2]);
        marker[-1] = marker[-2];
        return *this;
      }

      [[nodiscard]] inline bool has_pre_fx() const noexcept {
        J_ASSERT_NOT_NULL(marker, *marker, marker[-1], marker[-2]);
        J_ASSERT(marker[-1] >= marker[-2] && marker[-1] <= *marker);
        return marker[-1] != marker[-2];
      }

      [[nodiscard]] inline bool has_post_fx() const noexcept {
        J_ASSERT_NOT_NULL(marker, *marker, marker[-1], marker[-2]);
        J_ASSERT(marker[-1] >= marker[-2] && marker[-1] <= *marker);
        return marker[-1] != marker[0];
      }

      [[nodiscard]] inline span<expr*> pre_fx() const noexcept {
        J_ASSERT_NOT_NULL(marker, *marker, marker[-1], marker[-2]);
        J_ASSERT(marker[-1] >= marker[-2] && marker[-1] <= *marker);
        return {marker[-2], marker[-1]};
      }

      [[nodiscard]] inline span<expr*> post_fx() const noexcept {
        J_ASSERT_NOT_NULL(marker, *marker, marker[-1], marker[-2]);
        J_ASSERT(marker[-1] >= marker[-2] && marker[-1] <= *marker);
        return {marker[-1], marker[0]};
      }

      // [[nodiscard]] inline operator tsa_section() const noexcept {
      //   J_ASSERT_NOT_NULL(marker, *marker, split_point);
      //   return {marker - 2, marker};
      // }

      // inline split_tsa & inplace_concat_fx(const tsa_section & other) noexcept {
      //   J_ASSERT_NOT_NULL(marker, *marker, marker[-2]);
      //   if (other.begin == marker) {
      //     for (expr *** m = marker; m < other.end; ++m) {
      //       *m = *other.end;
      //     }
      //   } else {
      //     J_ASSERT(*other.end == marker[-2]);
      //     for (expr *** m = other.begin + 1; m <= marker - 2; ++m) {
      //       *m = *other.begin;
      //     }
      //   }
      //   return *this;
      // }

      inline split_tsa & inplace_concat_fx(split_tsa & other) noexcept {
        auto split = marker[-1];
        marker = stack->merge(marker, other.marker, 2);
        marker[-1] = split;
        J_ASSERT(marker[0] >= marker[-1] && marker[-1] >= marker[-2]);
        other.marker = nullptr;
        other.stack = nullptr;
        return *this;
      }

      inline split_tsa & inplace_concat_value(split_tsa & other) noexcept {
        auto split = other.marker[-1];
        marker = stack->merge(marker, other.marker, 2);
        marker[-1] = split;
        J_ASSERT_NOT_NULL(marker[0], marker[-1], marker[-2]);
        J_ASSERT(marker[0] >= marker[-1] && marker[-1] >= marker[-2]);
        other.marker = nullptr;
        other.stack = nullptr;
        return *this;
      }

      inline split_tsa & insert_middle_pre_fx(span<expr *> es) noexcept {
        stack->splice(marker - 1, es, true);
        return *this;
      }

      inline split_tsa & insert_middle_post_fx(span<expr *> es) noexcept {
        stack->splice(marker - 1, es, false);
        return *this;
      }

      template<typename T>
      split_tsa & push(T && e) noexcept {
        tsa_base::push(static_cast<T &&>(e));
        return *this;
      }

      [[nodiscard]] inline static split_tsa append_fx(split_tsa & lhs, split_tsa & rhs) noexcept {
        J_ASSERT(lhs.marker + 2 == rhs.marker);
        expr ** split = lhs.marker[-1];
        expr *** end = lhs.stack->merge(lhs.marker, rhs.marker, 2);
        end[-1] = split;
        lhs.marker = rhs.marker = nullptr;
        return split_tsa(lhs.stack, end);
      }

      [[nodiscard]] inline static split_tsa prepend_fx(split_tsa & lhs, split_tsa & rhs) noexcept {
        J_ASSERT(lhs.marker + 2 == rhs.marker);
        J_ASSERT(rhs.marker[-1] <= *rhs.marker);
        J_ASSERT(rhs.marker[-1] >= rhs.marker[-2]);
        expr ** split = rhs.marker[-1];
        expr *** end = lhs.stack->merge(lhs.marker, rhs.marker, 2);
        end[-1] = split;
        lhs.marker = rhs.marker = nullptr;
        return split_tsa(lhs.stack, end);
      }

      split_span release() noexcept {
        expr ** mid = marker[-1];
        auto span = tsa_base::release();
        return {span.begin(), mid, span.end()};
      }

      J_A(NODISC) operator split_span() const noexcept {
        check();
        return {marker[-2], marker[-1], marker[0]};
      }
    };

    J_A(NODISC,MU) inline term_stack_allocation allocate(term_stack & stack) noexcept {
      return {&stack, stack.allocate_marker()};
    }

    J_A(NODISC,MU) inline term_stack_allocation allocate(term_stack & stack, span<expr*> init) noexcept {
      ::j::memcpy(stack.markers[0], init.begin(), J_PTR_SZ * init.size());
      return {&stack, stack.allocate_marker(init.size())};
    }

    J_A(NODISC,MU) inline term_stack_allocation allocate(term_stack & stack, expr* J_NOT_NULL init) noexcept {
      return allocate(stack, span(&init, 1));
    }

    J_A(NODISC,MU) inline split_tsa allocate_split(term_stack & stack) noexcept {
      stack.allocate_marker();
      return {&stack, stack.allocate_marker()};
    }

    J_A(NODISC,MU) inline split_tsa allocate_split(term_stack & stack, span<expr*> init) noexcept {
      ::j::memcpy(stack.markers[0], init.begin(), J_PTR_SZ * init.size());
      stack.allocate_marker();
      return {&stack, stack.allocate_marker(init.size())};
    }

    J_A(NODISC,MU) inline split_tsa allocate_split(term_stack & stack, split_span init) noexcept {
      ::j::memcpy(stack.markers[0], init.begin(), J_PTR_SZ * init.size());
      stack.allocate_marker(init.num_pre_fx());
      return {&stack, stack.allocate_marker(init.num_post_fx())};
    }

    J_A(NODISC,MU) inline split_tsa allocate_split(term_stack & stack, expr* J_NOT_NULL init) noexcept {
      return allocate_split(stack, span(&init, 1));
    }

    template<i32_t NumMarkers>
    J_A(AI,HIDDEN) inline span<expr*> tsa_base<NumMarkers>::release() noexcept {
      span<expr*> result;
      if (marker) {
        check();
        result = stack->release_markers(marker, NumMarkers);
        stack = nullptr;
        marker = nullptr;
      }
      return result;
    }
  }
}
