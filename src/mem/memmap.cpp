#include "mem/memmap.hpp"
#include "files/fds/io.hpp"
#include "files/fds/fd.hpp"
#include "files/fds/open.hpp"
#include "files/paths/path.hpp"
#include "mem/buffer.hpp"

namespace j::mem {
  memory_mapping_info::memory_mapping_info(
    const void * begin,
    const void * end,
    u64_t offset,
    u64_t inode,
    bool is_readable,
    bool is_writable,
    bool is_executable,
    bool is_private,
    u8_t dev_major,
    u8_t dev_minor,
    strings::string && path) noexcept
    : m_begin(begin),
      m_end(end),
      m_offset(offset),
      m_inode(inode),
      m_is_readable(is_readable),
      m_is_writable(is_writable),
      m_is_executable(is_executable),
      m_is_private(is_private),
      m_dev_major(dev_major),
      m_dev_minor(dev_minor),
      m_path(static_cast<strings::string &&>(path))
  { }

  [[nodiscard]] trivial_array<memory_mapping_info> parse_mem_map(const char * J_NOT_NULL c);

  [[nodiscard]] trivial_array<memory_mapping_info> read_mem_maps() {
    auto maps = files::fds::open("/proc/self/maps", files::open_flags::read);
    J_ASSERT(maps);
    auto buf = files::fds::read_to_end(maps, 32);
    J_ASSERT(buf);
    return parse_mem_map((char*)buf.begin());
  }
}
