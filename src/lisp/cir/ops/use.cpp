#include "lisp/cir/ops/use.hpp"
#include "lisp/cir/ops/op.hpp"

namespace j::lisp::cir::inline ops {
  input * use::input() const noexcept {
    J_ASSUME_NOT_NULL(def);
    char * ptr = (char*)this;
    ptr -= sizeof(struct op);
    switch (type) {
    case use_type::none:
    case use_type::callee_saved:
    case use_type::op_result:
    case use_type::move:
      J_FAIL("Unsupported use type for input");
    case use_type::op_input:
      ptr -= J_OFFSET_OF_NON_STANDARD(op_result_input, use);
      ptr -= J_OFFSET_OF_NON_STANDARD(struct input, op_result_data);
      break;
    case use_type::mem_input_base:
      ptr -= J_OFFSET_OF_NON_STANDARD(op_result_input, use);
      ptr -= J_OFFSET_OF_NON_STANDARD(atomic_input, op_result_data);
      ptr -= J_OFFSET_OF_NON_STANDARD(mem_input, base);
      ptr -= J_OFFSET_OF_NON_STANDARD(struct input, mem_data);
      break;
    case use_type::mem_input_index:
      ptr -= J_OFFSET_OF_NON_STANDARD(op_result_input, use);
      ptr -= J_OFFSET_OF_NON_STANDARD(atomic_input, op_result_data);
      ptr -= J_OFFSET_OF_NON_STANDARD(mem_input, index);
      ptr -= J_OFFSET_OF_NON_STANDARD(struct input, mem_data);
      break;
    }
    return reinterpret_cast<struct input*>(ptr);
  }
  op * use::op() const noexcept {
    J_ASSUME_NOT_NULL(def);
    char * ptr = (char*)this;
    ptr -= sizeof(struct op);
    switch (type) {
    case use_type::none:
      J_FAIL("Empty path type");
    case use_type::callee_saved:
    case use_type::op_result:
      return def;
    case use_type::move:
      ptr -= sizeof(loc);
      break;
    case use_type::op_input:
      ptr -= J_OFFSET_OF_NON_STANDARD(op_result_input, use);
      ptr -= J_OFFSET_OF_NON_STANDARD(struct input, op_result_data);
      ptr -= input_index * sizeof(struct input);
      break;
    case use_type::mem_input_base:
      ptr -= J_OFFSET_OF_NON_STANDARD(op_result_input, use);
      ptr -= J_OFFSET_OF_NON_STANDARD(atomic_input, op_result_data);
      ptr -= J_OFFSET_OF_NON_STANDARD(mem_input, base);
      ptr -= J_OFFSET_OF_NON_STANDARD(struct input, mem_data);
      ptr -= input_index * sizeof(struct input);
      break;
    case use_type::mem_input_index:
      ptr -= J_OFFSET_OF_NON_STANDARD(op_result_input, use);
      ptr -= J_OFFSET_OF_NON_STANDARD(atomic_input, op_result_data);
      ptr -= J_OFFSET_OF_NON_STANDARD(mem_input, index);
      ptr -= J_OFFSET_OF_NON_STANDARD(struct input, mem_data);
      ptr -= input_index * sizeof(struct input);
      break;
    }
    return reinterpret_cast<struct op*>(ptr);
  }

  void use::detach() noexcept {
    J_ASSERT(!loc_in && !loc_out);
    J_ASSERT(def);
    previous_use()->next_use = next_use;
    reset();
  }

  void use::attach(use * J_NOT_NULL target, u8_t index, use_type type) noexcept {
    J_ASSUME(target != this);
    J_ASSUME(!next_use);
    next_use = target->next_use;
    target->next_use = this;
    input_index = index;
    this->type = type;
  }

  void use::attach(u8_t index, use_type type) noexcept {
    J_ASSUME_NOT_NULL(def);
    attach(&def->result, index, type);
  }

  use * use::previous_use() const noexcept {
    J_ASSERT(def);
    auto * prev = &def->result;
    while (prev->next_use != this) {
      prev = prev->next_use;
      J_ASSUME_NOT_NULL(prev);
    }
    return prev;
  }

  void use::set_next_use(struct use * J_NOT_NULL u) noexcept {
    J_ASSUME(u != this);
    J_ASSUME(!next_use);
    next_use = u;
  }
}
