#pragma once

#include "exceptions/assert_lite.hpp"

#include "hzd/utility.hpp"

namespace j::inline containers {
  struct dlist_link {
    dlist_link * next = nullptr;
    dlist_link * prev = nullptr;

    inline void unlink() noexcept {
      J_ASSERT(next && prev && next != this && prev != this && prev != next);
      next->prev = prev;
      prev->next = next;
    }

    inline void maybe_unlink() noexcept {
      if (next) {
        unlink();
      }
    }

    inline void link_after(dlist_link * J_NOT_NULL to) noexcept {
      J_ASSERT(to->next && to->prev && to->next != this && to->prev != this);
      prev = to;
      next = to->next;
      to->next = next->prev = this;
    }

    inline void link_before(dlist_link * J_NOT_NULL to) noexcept {
      J_ASSERT(to->next && to->prev && to->next != this && to->prev != this);
      next = to;
      prev = to->prev;
      to->prev = prev->next = this;
    }
  };

  template<auto MemPtr> struct dlist_iterator;

  template<typename T, typename Link, Link T::* MemPtr>
  struct dlist_iterator<MemPtr> final {
    Link * it = nullptr;

    J_INLINE_GETTER bool operator==(const dlist_iterator &) const noexcept = default;

    J_ALWAYS_INLINE dlist_iterator & operator++() noexcept {
      J_ASSERT_NOT_NULL(it);
      it = it->next;
      return *this;
    }

    J_ALWAYS_INLINE dlist_iterator operator++(int) noexcept {
      J_ASSERT_NOT_NULL(it);
      it = it->next;
      return {it->prev};
    }

    J_ALWAYS_INLINE dlist_iterator & operator--() noexcept {
      J_ASSERT_NOT_NULL(it);
      it = it->prev;
      return *this;
    }

    J_ALWAYS_INLINE dlist_iterator operator--(int) noexcept {
      J_ASSERT_NOT_NULL(it);
      it = it->prev;
      return {it->next};
    }

    J_INLINE_GETTER_NONNULL T * operator->() const noexcept {
      J_ASSERT(it && it->next && it->prev && it->next != it->prev);
      return subtract_bytes<T*>(it, (uptr_t)(MemPtr));
    }

    J_INLINE_GETTER T & operator*() const noexcept {
      return *operator->();
    }
  };

  template<auto MemPtr> struct dlist;

  template<typename T, dlist_link T::* MemPtr>
  struct dlist<MemPtr> final {
    J_BOILERPLATE(dlist, COPY_DEL, MOVE_DEL)

    dlist_link head;

    dlist() noexcept : head{&head, &head} { }

    J_INLINE_GETTER explicit operator bool() const noexcept { return head.next != head.prev; }
    J_INLINE_GETTER bool operator!() const noexcept { return head.next == head.prev; }
    J_INLINE_GETTER bool empty() const noexcept { return head.next == head.prev; }

    void clear() noexcept {
      head.next = head.prev = &head;
    }

    J_ALWAYS_INLINE void push_back(T & item) noexcept {
      (item.*MemPtr).link_before(&head);
    }

    J_ALWAYS_INLINE void push_front(T & item) noexcept {
      (item.*MemPtr).link_after(&head);
    }
  };
}
