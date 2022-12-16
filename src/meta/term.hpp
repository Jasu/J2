#pragma once

#include "meta/term_member.hpp"
#include "hzd/utility.hpp"
#include "containers/span.hpp"

namespace j::files::inline paths {
  class path;
}
namespace j::meta {
  struct term final : node {
    J_BOILERPLATE(term, CTOR_NE_ND, MOVE_NE_ND)
    term(strings::const_string_view name, doc_comment && comment) noexcept;

    void apply_defaults(const term & from) noexcept;

    term_member * ctor[6];
    term_member * storage[6];

    u8_t num_inputs = 0U;
    u8_t num_consts = 0U;
    u8_t num_data   = 0U;
    bool has_dynamic_size = false;
    bool has_dynamic_input = false;
    bool has_dynamic_const = false;
    bool has_dynamic_data = false;
    u8_t static_size = 0U;
    u8_t static_input_size = 0U;
    u8_t static_const_size = 0U;
    u8_t static_data_size = 0U;

    bool layout_equals(const term & other) const noexcept;

    J_A(RNN,NODISC) term_member * add_member(strings::const_string_view name,
                                             doc_comment && comment,
                                             val_region region,
                                             val_type * J_NOT_NULL type);

    [[nodiscard]] bool has_member(strings::const_string_view name) const noexcept;
    J_A(RNN,NODISC) term_member * get_member(strings::const_string_view name) noexcept;

    void initialize(const module * J_NOT_NULL mod);

    J_A(AI,ND,NODISC) inline span<term_member * const> inputs() noexcept {
      return span<term_member * const>(storage, num_inputs);
    }
    J_A(AI,ND,NODISC) inline span<const term_member * const> inputs() const noexcept {
      return span<const term_member * const>(storage, num_inputs);
    }

    J_A(AI,ND,NODISC) inline span<term_member * const> consts() noexcept {
      return span<term_member * const>(storage + num_inputs, num_consts);
    }
    J_A(AI,ND,NODISC) inline span<const term_member * const> consts() const noexcept {
      return span<const term_member * const>(storage + num_inputs, num_consts);
    }

    J_A(AI,ND,NODISC) inline span<term_member * const> data() noexcept {
      return span<term_member * const>(storage + num_consts + num_inputs, num_data);
    }
    J_A(AI,ND,NODISC) inline span<const term_member * const> data() const noexcept {
      return span<const term_member * const>(storage + num_consts + num_inputs, num_data);
    }

    J_A(AI,ND,NODISC) inline span<term_member * const> operands() noexcept {
      return span<term_member * const>(ctor, num_inputs + num_consts);
    }

    J_A(AI,ND,NODISC) inline span<const term_member * const> operands() const noexcept {
      return span<const term_member * const>(ctor, num_inputs + num_consts);
    }

    J_A(AI,ND,NODISC) inline span<term_member * const> all() noexcept {
      return span<term_member * const>(ctor, num_inputs + num_consts + num_data);
    }

    J_A(AI,ND,NODISC) inline span<const term_member * const> all() const noexcept {
      return span<const term_member * const>(ctor, num_inputs + num_consts + num_data);
    }

    J_A(AI,ND,NODISC) inline span<term_member * const> stored() noexcept {
      return span<term_member * const>(storage, num_inputs + num_consts + num_data);
    }

    J_A(AI,ND,NODISC) inline span<const term_member * const> stored() const noexcept {
      return span<const term_member * const>(storage, num_inputs + num_consts + num_data);
    }
  };
}
