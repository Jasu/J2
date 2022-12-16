#pragma once

#include "mem/control_block.hpp"

namespace j::mem {
  template<typename T> class shared_ptr;

  class shared_holder final {
  public:
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;

    J_A(AI,ND) inline shared_holder() noexcept = default;

    template<typename T>
    J_A(AI,ND,NE,HIDDEN) inline explicit shared_holder(const shared_ptr<T> & ptr) noexcept
      : shared_holder(ptr.holder)
    {
    }

    template<typename T>
    J_A(AI,ND,NE,HIDDEN) inline explicit shared_holder(shared_ptr<T> && ptr) noexcept
      : shared_holder(static_cast<shared_holder &&>(ptr.holder))
    {
    }

    J_A(AI,ND) inline explicit shared_holder(detail::shared_ptr_control_block * cb) noexcept
      : control_block(cb)
    { }

    shared_holder(const shared_holder & rhs) noexcept;

    J_A(AI,ND) inline shared_holder(shared_holder && rhs) noexcept : control_block(rhs.control_block) {
      rhs.control_block = nullptr;
    }

    shared_holder & operator=(const shared_holder & rhs) noexcept;

    shared_holder & operator=(shared_holder && rhs) noexcept;

    void reset() noexcept {
      if (control_block) {
        control_block->remove_reference();
        control_block = nullptr;
      }
    }

    inline ~shared_holder() {
      if (control_block) {
        control_block->remove_reference();
      }
    }

    J_A(AI,NODISC) inline bool operator!() const noexcept { return !control_block; }

    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return control_block; }

    detail::shared_ptr_control_block * control_block = nullptr;
  };
}
