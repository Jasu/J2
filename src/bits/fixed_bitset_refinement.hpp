#pragma once

#include "bits/fixed_bitset.hpp"
#include "containers/vector.hpp"
#include "containers/span.hpp"

namespace j::bits {
  template<typename Bitset>
  struct fixed_bitset_refinement final {
    vector<Bitset> partitions;

    void split(Bitset bitset) noexcept {
      for (u32_t max = partitions.size(), i = 0U; i < max; ++i) {
        if (auto intersection = partitions[i] & bitset) {
          bitset -= intersection;
          if (intersection != partitions[i]) {
            partitions[i] -= intersection;
            partitions.push_back(intersection);
          }
          if (!bitset) {
            return;
          }
        }
      }
      if (bitset) {
        partitions.push_back(bitset);
      }
    }

    vector<u32_t> get_refinement(const Bitset & bitset) const noexcept {
      vector<u32_t> result;
      u32_t i = 0U;
      for (auto & p : partitions) {
        if (p & bitset) {
          result.push_back(i);
        }
        ++i;
      }
      return result;
    }

    Bitset get_set(span<const u32_t> partition) const noexcept {
      Bitset result;
      for (u32_t i : partition) {
        result |= partitions[i];
      }
      return result;
    }
    J_A(AI,NODISC) inline const Bitset * begin() const noexcept { return partitions.begin(); }
    J_A(AI,NODISC) inline const Bitset * end() const noexcept { return partitions.end(); }
    J_A(AI,NODISC) inline u32_t size() const noexcept { return partitions.size(); }
    J_A(AI,NODISC) inline const Bitset & operator[](u32_t index) const noexcept { return partitions[index]; }
    J_A(AI,NODISC) inline const Bitset & at(u32_t index) const noexcept { return partitions.at(index); }
  };
}
