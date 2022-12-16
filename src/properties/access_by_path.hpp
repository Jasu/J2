#pragma once

namespace j::properties {
  namespace wrappers {
    class wrapper;
  }

  class path;

  namespace visiting {
    class visit_path;
  }

  enum class access_by_path_flag : unsigned int {
    create = 1,
    properties_as_keys = 2,
  };

  struct access_by_path_flags {
    unsigned int m_flags = 0;
  public:
    constexpr access_by_path_flags() noexcept = default;

    explicit constexpr access_by_path_flags(unsigned int flags) noexcept
      : m_flags(flags)
    {
    }

    constexpr access_by_path_flags(access_by_path_flag flag) noexcept
      : m_flags((unsigned int)flag)
    {
    }

    constexpr bool has(access_by_path_flag f) const noexcept {
      return (unsigned int)f & m_flags;
    }

    constexpr unsigned int flags() const noexcept {
      return m_flags;
    }
  };

  wrappers::wrapper access_by_path(wrappers::wrapper wrapper, const path & p, access_by_path_flags flags);

  wrappers::wrapper access_by_path(wrappers::wrapper wrapper, const visiting::visit_path & p, access_by_path_flags flags);

  constexpr inline access_by_path_flags operator|(access_by_path_flags lhs, access_by_path_flags rhs) noexcept {
    return access_by_path_flags(lhs.flags() | rhs.flags());
  }

  constexpr inline access_by_path_flags operator|(access_by_path_flag lhs, access_by_path_flags rhs) noexcept {
    return access_by_path_flags((unsigned int)lhs | rhs.flags());
  }

  constexpr inline access_by_path_flags operator|(access_by_path_flags lhs, access_by_path_flag rhs) noexcept {
    return access_by_path_flags(lhs.flags() | (unsigned int)rhs);
  }

  constexpr inline access_by_path_flags operator|(access_by_path_flag lhs, access_by_path_flag rhs) noexcept {
    return access_by_path_flags((unsigned int)lhs | (unsigned int)rhs);
  }
}
