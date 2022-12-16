#pragma once

#include "lisp/cir/locs/loc.hpp"
#include "lisp/cir/locs/loc_mask.hpp"

namespace j::lisp::assembly {
  class target;
}

namespace j::lisp::cir::inline locs {
  struct reg_map final {
    loc ordered_locs[32];
    loc_mask masks_by_loc_index[32];
    loc_mask available_mask;
    u8_t num_gprs = 0U;
    u8_t num_fp_regs = 0U;

    explicit reg_map(const assembly::target * J_NOT_NULL tgt) noexcept;

    reg_map(const reg_map &) = delete;

    [[nodiscard]] loc loc_by_mask(const loc_mask & m) const noexcept {
      for (u16_t i = 0; i < 32; ++i) {
        if (masks_by_loc_index[i] & m) {
          J_ASSERT(ordered_locs[i]);
          return ordered_locs[i];
        }
      }
      return {};
    }
  };
}
