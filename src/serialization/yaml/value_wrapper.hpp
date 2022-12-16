#pragma once

#include "hzd/types.hpp"

namespace j::properties::wrappers {
  class wrapper_definition;
}

namespace c4::yml {
  class Tree;
}

namespace j::serialization::yaml::detail {
  extern const properties::wrappers::wrapper_definition yaml_variant_wrapper_definition;

  /// Data contents of a single YAML variant.
  ///
  /// \note YAML map and list contain identical data as the variant - they have
  /// distinct types only to make different accesses have different type IDs.
  struct yaml_wrapper_data {
    ::c4::yml::Tree * tree;
    sz_t index;
  };

  /// Data contents of a YAML map.
  ///
  /// The map is not actually accessible as a map-like C++ object, only through
  /// property access. Thus, it's sufficient to use a POD object and do the hard
  /// work in the wrapper functions.
  ///
  /// \note A specific class is added solely to register the different property
  /// accesses with different type names.
  struct yaml_map : yaml_wrapper_data { };

  /// Data contents of a YAML sequence.
  ///
  /// The map is not actually accessible as a map-like C++ object, only through
  /// property access. Thus, it's sufficient to use a POD object and do the hard
  /// work in the wrapper functions.
  ///
  /// \note A specific class is added solely to register the different property
  /// accesses with different type names.
  struct yaml_list : yaml_wrapper_data { };
}
