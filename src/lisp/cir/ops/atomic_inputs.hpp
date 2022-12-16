#pragma once

#include "lisp/cir/ops/use.hpp"
#include "lisp/cir/locs/loc.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "lisp/common/id.hpp"
#include "lisp/functions/calling_convention.hpp"
#include "hzd/concepts.hpp"

namespace j::lisp::cir::inline ops {
  struct op;

  enum class atomic_input_type : u8_t {
    none = 0U,

    op_result,
    reloc,
    constant,
  };

  struct op_result_input final {
    J_BOILERPLATE(op_result_input, CTOR_CE)

    /// The instruction defining the result.
    use use{};

    constexpr explicit op_result_input(op * J_NOT_NULL def) noexcept
      : use{.def = def}
    { }

    constexpr op_result_input(op * J_NOT_NULL def, u8_t index, use_type type, struct loc loc = {}) noexcept
      : use{.type = type, .input_index = index, .loc_in = loc, .loc_out = loc, .next_use = nullptr, .def = def}
    { }

    constexpr op_result_input(op * J_NOT_NULL def, u8_t index, use_type type, struct loc in, struct loc out) noexcept
      : use{.type = type, .input_index = index, .loc_in = in, .loc_out = out, .next_use = nullptr, .def = def}
    { }

    J_INLINE_GETTER bool empty() const noexcept {
      return !use;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return (bool)use;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !use;
    }
  };

  enum class reloc_type : u8_t {
    none,

    constant_addr,
    fn_addr,
    var_addr,
    var_value_untagged,
    var_value_tagged,
  };

  struct reloc_input final {
    /// Name of the item to be relocated.
    union {
      id id;
      u32_t constant_offset;
    };
    /// Type of the relocation entry.
    reloc_type reloc_type;
    /// Calling convention when reloc_type is fn_addr.
    functions::calling_convention calling_convention;
    /// Bytes to add to the value.
    i32_t addend;

    reloc_input with_type(enum reloc_type type) const noexcept {
      return {{.constant_offset = constant_offset}, type, calling_convention, addend};
    }

    reloc_input with_addend(i32_t new_addend) const noexcept {
      return {{.constant_offset = constant_offset}, reloc_type, calling_convention, new_addend};
    }
  };

  constexpr reloc_input constant_reloc(u32_t offset, i32_t addend = 0) noexcept {
    return {
      {.constant_offset = offset},
      reloc_type::constant_addr,
      functions::calling_convention::none,
      addend,
    };
  }

  constexpr reloc_input full_call_reloc(id name, i32_t addend = 0) noexcept {
    return {
      {name},
      reloc_type::fn_addr,
      functions::calling_convention::full_call,
      addend,
    };
  }

  constexpr reloc_input abi_call_reloc(id name, i32_t addend = 0) noexcept {
    return {
      {name},
      reloc_type::fn_addr,
      functions::calling_convention::abi_call,
      addend
    };
  }

  constexpr reloc_input var_addr_reloc(id name, i32_t addend = 0) noexcept {
    return {
      {name},
      reloc_type::var_addr,
      functions::calling_convention::none,
      addend
    };
  }

  constexpr reloc_input var_value_untagged_reloc(id name, i32_t addend = 0) noexcept {
    return {
      {name},
      reloc_type::var_value_untagged,
      functions::calling_convention::none,
      addend,
    };
  }

  constexpr reloc_input var_value_tagged_reloc(id name, i32_t addend = 0) noexcept {
    return {
      {name},
      reloc_type::var_value_tagged,
      functions::calling_convention::none,
      addend,
    };
  }

  struct atomic_input final {
    J_BOILERPLATE(atomic_input, CTOR_CE)

    atomic_input(op * J_NOT_NULL o) noexcept;

    atomic_input(const op_result_input & result) noexcept;

    explicit constexpr atomic_input(u64_t constant, imm_type_mask types) noexcept
      : type(atomic_input_type::constant),
        types(types),
        const_data{constant}
    {
    }

    constexpr atomic_input(const reloc_input & reloc) noexcept
      : type(atomic_input_type::reloc),
        reloc_data{reloc}
    {
    }

    J_INLINE_GETTER bool is_op_result() const noexcept {
      return type == atomic_input_type::op_result;
    }

    J_INLINE_GETTER bool is_reloc() const noexcept {
      return type == atomic_input_type::reloc;
    }

    J_INLINE_GETTER bool is_fn_addr() const noexcept {
      return type == atomic_input_type::reloc && reloc_data.reloc_type == reloc_type::fn_addr;
    }

    J_INLINE_GETTER bool is_full_call_addr() const noexcept {
      return is_fn_addr() && reloc_data.calling_convention == functions::calling_convention::full_call;
    }

    J_INLINE_GETTER bool is_abi_call_addr() const noexcept {
      return is_fn_addr() && reloc_data.calling_convention == functions::calling_convention::abi_call;
    }

    J_INLINE_GETTER bool is_var_addr() const noexcept {
      return type == atomic_input_type::reloc && reloc_data.reloc_type == reloc_type::var_addr;
    }

    J_INLINE_GETTER bool is_tagged_var_value() const noexcept {
      return type == atomic_input_type::reloc && reloc_data.reloc_type == reloc_type::var_value_tagged;
    }

    J_INLINE_GETTER bool is_untagged_var_value() const noexcept {
      return type == atomic_input_type::reloc && reloc_data.reloc_type == reloc_type::var_value_untagged;
    }

    J_INLINE_GETTER bool is_constant() const noexcept {
      return type == atomic_input_type::constant;
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return type == atomic_input_type::none;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return type != atomic_input_type::none;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return type == atomic_input_type::none;
    }

    atomic_input_type type = atomic_input_type::none;
    imm_type_mask types{};
    union {
      op_result_input op_result_data = {};
      reloc_input reloc_data;
      u64_t const_data;
    };
  };

  constexpr inline atomic_input const_true{true_v, imm_bool};
  constexpr inline atomic_input const_false{false_v, imm_bool};
  constexpr inline atomic_input const_nil{nil_v, imm_nil};

  template<Integral T>
  [[nodiscard]] inline constexpr atomic_input untagged_raw_const(T value, imm_type_mask types) noexcept {
    return atomic_input(value, types);
  }

  template<Integral T>
  [[nodiscard]] inline constexpr atomic_input tagged_raw_const(T value, imm_type_mask types) noexcept {
    return atomic_input(value, types);
  }

  template<Integral T>
  [[nodiscard]] inline constexpr atomic_input untagged_const_int(T value) noexcept {
    return atomic_input(value, imm_i64);
  }

  [[nodiscard]] inline constexpr const atomic_input & const_bool(bool value) noexcept {
    return value ? const_true : const_false;
  }

}
