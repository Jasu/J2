#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::util::detail {
  template<typename Type, bool PassParent>
  struct context_stack_entry {
    mutable Type value;
    context_stack_entry * parent = nullptr;
    J_BOILERPLATE(context_stack_entry, CTOR_NE_ND, MOVE_NE_ND)

    template<typename T>
    J_A(AI) inline explicit context_stack_entry(T && value) noexcept
      : value(static_cast<T &&>(value))
    {
    }

    template<typename... Args>
    J_A(AI,ND) inline explicit context_stack_entry(context_stack_entry * parent, Args && ... args) noexcept
      : value(static_cast<Args &&>(args)...),
        parent(parent)
    {
    }
  };

  template<typename Type>
  struct context_stack_entry<Type, true> {
    mutable Type value;
    context_stack_entry * parent = nullptr;
    J_BOILERPLATE(context_stack_entry, CTOR_NE_ND, MOVE_NE_ND)

    template<typename T>
    J_A(AI) inline explicit context_stack_entry(T && value) noexcept
      : value(static_cast<T &&>(value))
    {
    }

    template<typename... Args>
    J_A(AI,ND) inline explicit context_stack_entry(context_stack_entry * parent, Args && ... args) noexcept
      : value(parent ? &parent->value : nullptr, static_cast<Args &&>(args)...),
        parent(parent)
    {
    }
  };
}

namespace j::util {
  template<typename Type, bool PassParent = false>
  struct context_stack final {
    using entry_t J_NO_DEBUG_TYPE = j::util::detail::context_stack_entry<Type, PassParent>;

    bool has_default = false;
    u64_t default_entry[(sizeof(entry_t) + sizeof(u64_t) - 1) / sizeof(u64_t)] = { 0 };
    entry_t * current = nullptr;

    J_A(AI,ND) constexpr inline context_stack() noexcept = default;

    J_A(AI,ND) inline explicit constexpr context_stack(Type && default_value) noexcept
      : has_default(true),
        current(reinterpret_cast<entry_t*>(&default_entry))
    {
      ::new (&default_entry) entry_t(static_cast<Type &&>(default_value));
    }

    J_A(AI,ND,HIDDEN,NODISC) inline bool has_value() const noexcept {
      return current;
    }

    J_A(AI,ND,HIDDEN,NODISC) inline Type * maybe_get() const noexcept {
      return current ? &current->value : nullptr;
    }

    J_INLINE_GETTER Type & get() const noexcept {
      J_ASSUME_NOT_NULL(current);
      return current->value;
    }

    J_A(AI,HIDDEN,NODISC,RNN) Type * operator->() const noexcept {
      return &current->value;
    }

    J_A(AI,HIDDEN,NODISC) inline Type & operator*() const noexcept {
      return current->value;
    }

    struct guard final : entry_t {
      context_stack * stack = nullptr;

      J_A(AI,ND) inline guard() noexcept = default;

      J_A(ND) guard(const guard &) = delete;

      J_A(ND) guard(guard && rhs) noexcept
        : entry_t(static_cast<entry_t &&>(rhs)),
          stack(rhs.stack)
      {
        if (J_UNLIKELY(!stack)) {
          return;
        }
        rhs.stack = nullptr;
        stack->current = this;
      }

      J_A(ND) guard & operator=(guard && rhs) noexcept {
        if (J_LIKELY(this != &rhs)) {
          stack = rhs.stack;
          if (J_LIKELY(stack)) {
            *static_cast<entry_t*>(this) = static_cast<entry_t &&>(rhs);
            rhs.stack = nullptr;
            stack->current = this;
          }
        }
        return *this;
      }

      template<typename... Ts>
      J_A(AI,ND,HIDDEN) inline explicit guard(context_stack * J_NOT_NULL stack, Ts && ...args) noexcept
        : entry_t(stack->current, static_cast<Ts &&>(args)...),
          stack(stack)
      {
        stack->current = this;
      }

      ~guard() {
        if (stack) {
          stack->current = this->parent;
        }
      }
    };

    template<typename... Ts>
    J_A(AI,ND,NODISC,HIDDEN) inline guard enter(Ts && ...args) noexcept {
      return guard(this, static_cast<Ts &&>(args)...);
    }
  };
}
