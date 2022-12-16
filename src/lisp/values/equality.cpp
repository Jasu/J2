#include "equality.hpp"
#include "lisp/values/lisp_vec.hpp"
#include "lisp/values/lisp_str.hpp"

namespace j::lisp::inline  values {
  [[nodiscard]] bool shallow_vec_eq(lisp_imm lhs, lisp_imm rhs) noexcept {
    if (eq(lhs, rhs)) {
      return true;
    }
    if (lhs.type() != imm_vec_ref || rhs.type() != imm_vec_ref) {
      return false;
    }
    auto vl = lhs.as_vec_ref()->value(), vr = rhs.as_vec_ref()->value();
    u32_t sz = vl.size();
    if (sz != vr.size()) {
      return false;
    }
    for (u32_t i = 0U; i < sz; ++i) {
      if (!eq(vl[i], vr[i])) {
        return false;
      }
    }
    return true;
  }

}
