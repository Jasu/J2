#pragma once

#include "files/paths/path_map_fwd.hpp"
#include "containers/hash_map.hpp"

#define J_DEFINE_EXTERN_PATH_MAP(VALUE) J_DEFINE_EXTERN_HASH_MAP(j::files::path, VALUE, HASH(j::files::paths::path_hash))
