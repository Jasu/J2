#pragma once

#include "containers/obstack_fwd.hpp"
#include "mem/shared_holder.hpp"

J_DECLARE_EXTERN_OBSTACK(j::mem::shared_holder);
extern template j::mem::shared_holder & j::containers::obstack<j::mem::shared_holder>::emplace(j::mem::shared_ptr<void> &&);

namespace j::services::detail {
  using dependencies_t = containers::obstack<mem::shared_holder>;
}
