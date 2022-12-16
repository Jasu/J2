#pragma once

#include "hzd/types.hpp"

namespace j::type_id {
  struct type_id;
}

namespace j::properties::wrappers {
  class wrapper_definition;
  class wrapper;
}

namespace j::properties::access {
  class typed_access;
  class typed_access_definition;
}

namespace j::properties::conversions {
  class conversion_definition;

  class conversion_collection final {
    /// Type-erased hash_map
    sz_t m_map[3];
  public:
    conversion_collection() noexcept;
    conversion_collection(conversion_collection && rhs) noexcept;
    conversion_collection(const conversion_collection & rhs);
    conversion_collection & operator=(conversion_collection && rhs) noexcept;
    conversion_collection & operator=(const conversion_collection & rhs);
    ~conversion_collection();

    void reserve(sz_t sz);

    bool can_convert_from(const access::typed_access_definition & type) const noexcept;

    wrappers::wrapper create_from(
      const access::typed_access & from,
      const wrappers::wrapper_definition & def) const;

    void assign_from(
      access::typed_access & target,
      const access::typed_access & from) const;

    void add_conversion_from(const type_id::type_id & type, const struct conversion & coversion);
    void add_conversion_from_(const conversion_definition & coversion);

    void remove_conversion_from(const type_id::type_id & type);
  };
}
