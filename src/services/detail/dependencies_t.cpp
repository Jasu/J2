#include "services/detail/dependencies_t.hpp"
#include "containers/obstack.hpp"
#include "mem/shared_ptr_fwd.hpp"

J_DEFINE_EXTERN_OBSTACK(j::mem::shared_holder);
template j::mem::shared_holder & j::containers::obstack<j::mem::shared_holder>::emplace(j::mem::shared_ptr<void> &&);
