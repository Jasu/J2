#pragma once

#include "serialization/backend.hpp"

namespace j::serialization::yaml {
  class yaml_backend final : public backend {
  public:
    bool supports(const files::path & path) const override;

    properties::wrappers::stored_wrapper read(const files::path & path) const override;
  };
}
