#pragma once

namespace j::properties::access {
  class typed_access;
}

namespace j::properties::wrappers {
  class wrapper_definition;
  class wrapper;
}

namespace j::properties::conversions {
  struct conversion {
    void * (*m_create_from)(const void * source, void * data) = nullptr;
    void (*m_create_from_in_place)(void * target, const void * source, void * data) = nullptr;
    void (*m_assign_from)(void * target, const void * source, void * data) = nullptr;
    void * m_data = nullptr;

    wrappers::wrapper create_from(const wrappers::wrapper_definition & def, const void * source) const;

    void assign_from(access::typed_access & target, const void * source) const;
  };
}
