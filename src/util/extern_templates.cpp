#include "strings/string.hpp"
#include "containers/trivial_array.hpp"
#include "containers/vector.hpp"
#include "mem/shared_ptr.hpp"
#include "files/paths/path.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY_COPYABLE(i32_t);
J_DEFINE_EXTERN_TRIVIAL_ARRAY_COPYABLE(::j::strings::string);
J_DEFINE_EXTERN_VECTOR(u32_t);
J_DEFINE_EXTERN_VECTOR(j::strings::const_string_view);
J_DEFINE_EXTERN_VECTOR(j::strings::string);
J_DEFINE_EXTERN_VECTOR(j::mem::shared_ptr<void>);
J_DEFINE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::files::path);
