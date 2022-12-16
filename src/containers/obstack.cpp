#include "containers/obstack.hpp"

namespace j::detail {
  obstack_chunk * allocate_obstack_chunk(u32_t chunk_size, obstack_chunk * next) {
      obstack_chunk * result = (obstack_chunk*)allocate(sizeof(obstack_chunk) - 1 + chunk_size);
      result->size = 0, result->next = next;
      return result;
    }

  obstack_chunk * release_obstack_chunk(obstack_chunk * J_NOT_NULL chunk) noexcept {
    obstack_chunk * result = chunk->next;
    ::j::free(chunk);
    return result;
  }

}
