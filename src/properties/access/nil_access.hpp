#pragma once

#include "properties/access/typed_access.hpp"

namespace j::properties::access {
  class nil_access_definition : public typed_access_definition { };

  class nil_access final : public typed_access {
  public:
    using typed_access::typed_access;
  };
}
