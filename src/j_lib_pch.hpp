// Precompiled header pasted on top of each translation unit in the J library.
// Include the headers in sources still, as not to dependn on non-standard
// compiler features entirely too much.

#include <vulkan/vulkan.h>
#include <typeinfo>

#include "hzd/mem.hpp"
#include "hzd/integer_traits.hpp"
#include "hzd/string.hpp"

#include "exceptions/assert_lite.hpp"

#include "util/flags.hpp"

#include "mem/shared_ptr.hpp"

#include "strings/string.hpp"
#include "strings/format.hpp"
#include "strings/styling/styled_string.hpp"

#include "attributes/attribute_definition.hpp"
#include "attributes/attribute_map.hpp"

#include "services/metadata.hpp"
#include "properties/metadata.hpp"
#include "properties/access/registry.hpp"

#include "graphviz/attributes.hpp"
