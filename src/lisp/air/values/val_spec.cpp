#include "val_spec.hpp"

namespace j::lisp::air::inline values {
  const val_spec val_none{};
  const val_spec val_any{any_imm_type,               rep_spec::any};

  const val_spec val_range{range_imm_type,           rep_spec::range};
  const val_spec val_vec_or_range{range_imm_type,    rep_spec::any};

  const val_spec val_vec{imm_vec_ref,                rep_spec::any};

  const val_spec val_imm{taggable_imm_type,          rep_spec::any};
  const val_spec val_imm_tag{taggable_imm_type,      rep_spec::tagged};
}
