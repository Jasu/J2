#include "mem/shared_holder.hpp"

namespace j::mem {
    shared_holder::shared_holder(const shared_holder & rhs) noexcept : control_block(rhs.control_block) {
      if (control_block) {
        ++control_block->num_references;
      }
    }
    shared_holder & shared_holder::operator=(const shared_holder & rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (control_block) {
          control_block->remove_reference();
        }
        control_block = rhs.control_block;
        if (control_block) {
          ++control_block->num_references;
        }
      }
      return *this;
    }

    shared_holder & shared_holder::operator=(shared_holder && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        if (control_block) {
          control_block->remove_reference();
        }
        control_block = rhs.control_block;
        rhs.control_block = nullptr;
      }
      return *this;
    }

}
