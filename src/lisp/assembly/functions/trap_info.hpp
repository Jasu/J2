#pragma once

#include "lisp/assembly/registers.hpp"
#include "lisp/values/imm_type.hpp"
#include "lisp/common/id.hpp"
#include "lisp/sources/source_location.hpp"

namespace j::lisp::assembly::inline functions {
  enum class trap_type : u8_t {
    none,

    type_check,
    error,
    debug,
  };

  struct alignas(8) trap_info final {
    u32_t offset = 0U;
    trap_type type = trap_type::none;
    reg reg = {};
    imm_type_mask expected_type = {};
    id var_name = {};
    sources::source_location loc = {};
  };
}
