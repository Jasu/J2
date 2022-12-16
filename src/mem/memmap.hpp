#pragma once

#include "hzd/types.hpp"
#include "strings/string.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::mem {
  class memory_mapping_info final {
  public:
    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;

    J_ALWAYS_INLINE constexpr memory_mapping_info() noexcept = default;

    memory_mapping_info(const void * begin,
                        const void * end,
                        u64_t offset,
                        u64_t inode,
                        bool is_readable,
                        bool is_writable,
                        bool is_executable,
                        bool is_private,
                        u8_t dev_major,
                        u8_t dev_minor,
                        strings::string && path) noexcept;

    J_INLINE_GETTER const void * begin()           const noexcept { return m_begin; }
    J_INLINE_GETTER const void * end()             const noexcept { return m_end; }
    J_INLINE_GETTER u64_t offset()                 const noexcept { return m_offset; }
    J_INLINE_GETTER u64_t inode()                  const noexcept { return m_inode; }
    J_INLINE_GETTER bool is_readable()             const noexcept { return m_is_readable; }
    J_INLINE_GETTER bool is_writable()             const noexcept { return m_is_writable; }
    J_INLINE_GETTER bool is_executable()           const noexcept { return m_is_executable; }
    J_INLINE_GETTER bool is_private()              const noexcept { return m_is_private; }
    J_INLINE_GETTER bool is_shared()               const noexcept { return !m_is_private; }
    J_INLINE_GETTER u8_t dev_major()               const noexcept { return m_dev_major; }
    J_INLINE_GETTER u8_t dev_minor()               const noexcept { return m_dev_minor; }
    J_INLINE_GETTER const strings::string & path() const noexcept { return m_path; }
  private:
    const void * m_begin = nullptr;
    const void * m_end = nullptr;
    u64_t m_offset = 0ULL;
    u64_t m_inode = 0U;
    bool m_is_readable = false;
    bool m_is_writable = false;
    bool m_is_executable = false;
    bool m_is_private = false;
    u8_t m_dev_major = 0U;
    u8_t m_dev_minor = 0U;
    strings::string m_path;
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::mem::memory_mapping_info);

namespace j::mem {
  [[nodiscard]] trivial_array<memory_mapping_info> read_mem_maps();
}
