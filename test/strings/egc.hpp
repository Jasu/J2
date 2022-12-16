#include <detail/preamble.hpp>

#include "strings/unicode/utf8_rope.hpp"
#include "strings/unicode/utf8_rope_egc_iterator.hpp"
#include "rope.hpp"

inline void assert_grapheme_cluster(const s::utf8_rope_view & v, s::const_string_view expected) {
  char cluster[v.size()];
  v.copy_to(cluster, v.size());
  s::const_string_view cluster_view(cluster, v.size());
  REQUIRE(cluster_view == expected);
}

template<u32_t N>
void assert_grapheme_clusters(const s::utf8_rope & r, const char * const (&clusters)[N]) {
  u32_t i = 0U;
  for (auto v : r.grapheme_clusters()) {
    assert_grapheme_cluster(v, clusters[i]);
    REQUIRE(i < N);
    ++i;
  }
  REQUIRE(i == N);
}
