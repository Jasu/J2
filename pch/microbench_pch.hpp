// Precompiled header pasted on top of each translation unit in the micro benchmarks.
// Include the headers in sources still, as not to dependn on non-standard
// compiler features entirely too much.

#include <typeinfo>
#include <string>
#include <map>

#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"
#include "hzd/string.hpp"

#include "mem/shared_ptr.hpp"

#include "strings/string.hpp"
#include "strings/string_view.hpp"

#include "attributes/attribute_definition.hpp"
#include "attributes/attribute_map.hpp"

#include "ext/robin_hood.h"
#include "preamble.hpp"
