#include "lisp/cir/ops/atomic_inputs.hpp"
#include "lisp/cir/ops/op.hpp"

namespace j::lisp::cir::inline ops {
  atomic_input::atomic_input(op * J_NOT_NULL o) noexcept
    : type(atomic_input_type::op_result),
      types(o->result_types),
      op_result_data{o}
  {
  }
  atomic_input::atomic_input(const op_result_input & result) noexcept
      : type(atomic_input_type::op_result),
        types(result.use.def->result_types),
        op_result_data{result}
    {
    }

  input::input(const atomic_input & i) noexcept
    : type((input_type)i.type),
      types(i.types)
  {
    switch (i.type) {
    case atomic_input_type::none:
      break;
    case atomic_input_type::op_result:
      op_result_data = i.op_result_data;
      break;
    case atomic_input_type::reloc:
      reloc_data = i.reloc_data;
      break;
    case atomic_input_type::constant:
      const_data = i.const_data;
      break;
    }
  }

  void input::attach(u8_t index) noexcept {
    if (type == input_type::op_result) {
      op_result_data.use.attach(index);
    } else if (type == input_type::mem) {
      if (mem_data.base.is_op_result()) {
        mem_data.base.op_result_data.use.attach(index, use_type::mem_input_base);
      }
      if (mem_data.index.is_op_result()) {
        mem_data.index.op_result_data.use.attach(index, use_type::mem_input_index);
      }
    }
  }

  void input::detach() noexcept {
    if (type == input_type::op_result) {
      op_result_data.use.detach();
    } else if (type == input_type::mem) {
      if (mem_data.base.is_op_result()) {
        mem_data.base.op_result_data.use.detach();
      }
      if (mem_data.index.is_op_result()) {
        mem_data.index.op_result_data.use.detach();
      }
    }
  }

  input input::copy_value() noexcept {
    input result(*this);
    if (result.is_op_result()) {
      result.op_result_data.use.next_use = nullptr;
    } else if (result.is_mem()) {
      if (result.mem_data.base.is_op_result()) {
        result.mem_data.base.op_result_data.use.next_use = nullptr;
      }
      if (result.mem_data.index.is_op_result()) {
        result.mem_data.index.op_result_data.use.next_use = nullptr;
      }
    }
    return result;
  }

  void input::set_value(u8_t index, const input & in) noexcept {
    detach();
    ::j::memcpy(this, &in, sizeof(input));
    attach(index);
  }

  void input::verify_attached(op * J_NOT_NULL target) noexcept {
    if (is_op_result()) {
      J_ASSERT(op_result_data.use.op() == target);
    } else if (is_mem()) {
      if (mem_data.base.is_op_result()) {
        J_ASSERT(mem_data.base.op_result_data.use.op() == target);
      }
      if (mem_data.index.is_op_result()) {
        J_ASSERT(mem_data.index.op_result_data.use.op() == target);
      }
    }
  }

}
