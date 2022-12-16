#include "files/memory_mapping.hpp"
#include "files/fds/fd.hpp"
#include "files/fds/stat.hpp"
#include "files/fds/open.hpp"
#include "exceptions/exceptions.hpp"
#include "hzd/mem.hpp"
#include "mem/shared_ptr.hpp"

#include <sys/mman.h>

namespace j::files {
  namespace {
    J_RETURNS_NONNULL char * map(const fds::fd & fd, i32_t start_offset, i32_t size) {
      J_ASSERT(size > 0 && start_offset >= 0);
      J_REQUIRE(fd.can_read(), "Cannot map a closed or unreadable FD.");
      char * mapping = reinterpret_cast<char *>(::mmap(nullptr, page_align_up(size), PROT_READ, MAP_PRIVATE, (int)fd, start_offset));
      if (mapping == MAP_FAILED) {
        J_THROW(bad_alloc_exception() << message("mmap failed."));
      }
      return mapping;
    }
  }

  memory_mapping::memory_mapping(const fds::fd & fd, i32_t start_offset, i32_t size)
    : mapping(map(fd, start_offset, size), size)
  {
  }

  memory_mapping::memory_mapping() noexcept = default;

  memory_mapping::memory_mapping(const fds::fd & fd, map_full_range_t)
    : memory_mapping(fd, 0, fds::file_size(fd))
  {
  }

  memory_mapping::memory_mapping(strings::const_string_view path, i32_t start_offset, i32_t size)
    : memory_mapping(fds::open(path, open_flags::read), start_offset, size)
  { }

  memory_mapping::memory_mapping(strings::const_string_view path, map_full_range_t)
    : memory_mapping(fds::open(path, open_flags::read), map_full_range)
  { }

  memory_mapping::memory_mapping(const path & path, i32_t start_offset, i32_t size)
    : memory_mapping(fds::open(path, open_flags::read), start_offset, size)
  { }

  memory_mapping::memory_mapping(const path & path, map_full_range_t)
    : memory_mapping(fds::open(path, open_flags::read), map_full_range)
  { }

  memory_mapping & memory_mapping::operator=(memory_mapping && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      if (J_UNLIKELY(mapping.m_data)) {
        unmap();
      }
      mapping = rhs.mapping;
      rhs.mapping.reset();
    }
    return *this;
  }

  void memory_mapping::unmap() noexcept {
    J_ASSERT_NOT_NULL(mapping.m_data);
    [[maybe_unused]] int result = ::munmap((void*)mapping.m_data, page_align_up(mapping.m_size));
    J_ASSERT(!result, "munmap failed.");
    mapping.reset();
  }

  memory_mapping::~memory_mapping() {
    if (mapping) {
      unmap();
    }
  }
}
