#pragma once

#include "mem/r_buffer.hpp"
#include "strings/string_view.hpp"

namespace j::files {
  inline namespace fds {
    struct fd;
  }
  inline namespace paths {
    class path;
  }

  enum map_full_range_t { map_full_range };

  /// Memory mapping of a region from a file.
  ///
  /// \todo Handle SIGBUS
  struct memory_mapping final {
    memory_mapping() noexcept;

    memory_mapping(strings::const_string_view p, i32_t start_offset, i32_t size);

    memory_mapping(strings::const_string_view p, map_full_range_t);

    memory_mapping(const path & p, i32_t start_offset, i32_t size);

    memory_mapping(const path & p, map_full_range_t);

    memory_mapping(const fds::fd & fd, i32_t start_offset, i32_t size);

    memory_mapping(const fds::fd & fd, map_full_range_t);

    J_A(AI,ND,HIDDEN) inline memory_mapping(memory_mapping && rhs) noexcept
      : mapping(rhs.mapping)
    { rhs.mapping.reset(); }

    memory_mapping & operator=(memory_mapping &&) noexcept;

    void unmap() noexcept;

    ~memory_mapping();

    J_A(AI,NODISC,ND,HIDDEN,PGALIGN) inline const char * data() const noexcept {
      return mapping.m_data;
    }

    J_A(AI,NODISC,ND,HIDDEN) inline i32_t size() const noexcept {
      return mapping.size();
    }

    j::mem::r_buffer mapping;
  };
}
