#include "hzd/mem.hpp"
#include "mem/shared_ptr.hpp"
#include "mem/weak_ptr.hpp"
#include "exceptions/assert.hpp"

template class j::mem::shared_ptr<void>;
template class j::mem::weak_ptr<void>;

namespace j::mem {
  namespace detail {
    void shared_ptr_control_block::remove_last_reference() noexcept {
      if (destructor) {
        destructor(target);
      }
      J_ASSUME(num_references == 1);
      if (!num_weak_references) {
        ::operator delete (static_cast<void*>(this));
      } else {
        target = nullptr;
        num_references = 0;
        destructor = nullptr;
      }
    }

    J_A(RNN,NODISC) shared_ptr_control_block * allocate_control_block(u32_t object_size, deleter_t destructor) noexcept {
      object_size = align_up(object_size, 8U);
      auto buf = ::operator new (object_size + sizeof(detail::shared_ptr_control_block));
      auto cb = reinterpret_cast<shared_ptr_control_block*>(buf);
      cb->num_references = 1U;
      cb->num_weak_references = 0U;
      cb->destructor = destructor;
      cb->target = object_size
        ? reinterpret_cast<char *>(buf) + sizeof(detail::shared_ptr_control_block)
        : nullptr;
      return cb;
    }

  }
}
