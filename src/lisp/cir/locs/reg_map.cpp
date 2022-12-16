#include "lisp/cir/locs/reg_map.hpp"
#include "lisp/assembly/target.hpp"
#include "lisp/assembly/register_info.hpp"

namespace j::lisp::cir::inline locs {
  namespace {
    loc handle_reg(reg_map * J_NOT_NULL to, assembly::phys_reg reg, u32_t index)
    {
      J_ASSERT(reg);
      const loc l = loc::from_phys_reg(reg);
      J_ASSERT(l && l.is_reg() && l.as_phys_reg() == reg);
      J_ASSERT(!to->ordered_locs[index]);
      to->ordered_locs[index] = l;
      J_ASSERT(to->masks_by_loc_index[index].empty());
      to->masks_by_loc_index[index].add(l);
      return l;
    }
  }
  reg_map::reg_map(const assembly::target * J_NOT_NULL tgt) noexcept {
    auto ri = tgt->get_reg_info();
    for (auto reg : ri->gpr_alloc_list) {
      const loc l = handle_reg(this, reg, num_gprs++);
      J_ASSERT(l && l.is_gpr());
      available_mask.add(l);
    }

    for (auto reg : ri->fp_alloc_list) {
      const loc l = handle_reg(this, reg, 16 + num_fp_regs++);
      J_ASSERT(l && l.is_fp());
      available_mask.add(l);
    }
  }
}
