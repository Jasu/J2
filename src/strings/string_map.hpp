#include "strings/string_map_fwd.hpp"
#include "containers/hash_map.hpp"

#define J_DEFINE_EXTERN_STRING_MAP(V) J_DEFINE_EXTERN_HASH_MAP(j::strings::string, V, HASH(j::strings::string_hash), CONST_KEY(j::strings::string_const_key))
