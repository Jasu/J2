#pragma once

#include "hzd/concepts.hpp"

namespace j::inline containers::trees {
  template<typename DescriptionT>
  concept RopeTreeDescription = requires {
    /// The primary "key" / length type.
    typename DescriptionT::key_t;
    /// Signed difference type for key_t.
    typename DescriptionT::key_diff_t;
    typename DescriptionT::value_t;
    /// Container for aggregate metrics for value spans. Must contain key_t.
    typename DescriptionT::metrics_t;
    typename DescriptionT::controller_t;
    { DescriptionT::max_size_v } -> SameAs<const u8_t &>;
  };

  template<RopeTreeDescription Description> class rope_tree_node;
}
