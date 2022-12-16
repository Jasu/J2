#pragma once

#include "mem/memory_region.hpp"

namespace j::mem {
  namespace detail {
    enum class allocate_tag { v };
    enum class take_ownership_tag { v };
    enum class no_take_ownership_tag { v };
  }

  constexpr inline detail::allocate_tag allocate_tag{detail::allocate_tag::v};
  constexpr inline detail::take_ownership_tag take_ownership_tag{detail::take_ownership_tag::v};
  constexpr inline detail::no_take_ownership_tag no_take_ownership_tag{detail::no_take_ownership_tag::v};

  namespace detail {
    template<typename Ptr>
    class basic_buffer_base : public basic_memory_region<Ptr> {
    public:
      /// Construct an empty buffer.
      J_ALWAYS_INLINE constexpr basic_buffer_base() noexcept = default;

      /// Move-construct a buffer. Rhs will be left empty.
      J_ALWAYS_INLINE constexpr basic_buffer_base(basic_buffer_base && rhs) noexcept
        : basic_memory_region<Ptr>(rhs),
          m_owns_buffer(rhs.m_owns_buffer)
      {
        rhs.m_owns_buffer = false;
        rhs.reset();
      }

      /// Construct a buffer, allocating it.
      J_ALWAYS_INLINE constexpr basic_buffer_base(detail::allocate_tag, u32_t size) noexcept
        : basic_memory_region<Ptr>(size ? (Ptr)j::allocate_zero(size) : nullptr, size),
          m_owns_buffer(true)
      { }

      /// Construct a buffer that owns its contents.
      J_ALWAYS_INLINE constexpr basic_buffer_base(detail::take_ownership_tag, void * J_NOT_NULL data, u32_t size) noexcept
        : basic_memory_region<Ptr>((Ptr)data, size),
          m_owns_buffer(true)
      { }

      /// Construct a buffer that does not own its contents.
      J_ALWAYS_INLINE constexpr basic_buffer_base(detail::no_take_ownership_tag, void * J_NOT_NULL data, u32_t size) noexcept
        : basic_memory_region<Ptr>((Ptr)data, size),
          m_owns_buffer(false)
      { }

      /// Move-assign to a buffer. Rhs will be left empty.
      constexpr basic_buffer_base & operator=(basic_buffer_base && rhs) noexcept {
        if (J_LIKELY(this != &rhs)) {
          reset();
          basic_memory_region<Ptr>::operator=(rhs);
          m_owns_buffer = rhs.m_owns_buffer;
          rhs.m_owns_buffer = false;
          rhs.reset();
        }
        return *this;
      }
      J_INLINE_GETTER bool owns_buffer() const noexcept       { return m_owns_buffer; }

      J_ALWAYS_INLINE ~basic_buffer_base()                    { reset(); }

      J_ALWAYS_INLINE void reset() noexcept {
        if (m_owns_buffer) {
          ::j::free(const_cast<char*>(basic_memory_region<Ptr>::begin()));
        }
        basic_memory_region<Ptr>::reset();
      }
    protected:
      void set(basic_memory_region<Ptr> && region) {
        basic_memory_region<Ptr>::operator=(static_cast<basic_memory_region<Ptr> &&>(region));
      }
    private:
      bool m_owns_buffer = false;
    };
  }

  template<typename Ptr>
  class basic_buffer final : public basic_memory_region<Ptr> {
  public:
    /// Construct an empty buffer.
    J_ALWAYS_INLINE constexpr basic_buffer() noexcept = default;

    /// Move-construct a buffer. Rhs will be left empty.
    J_ALWAYS_INLINE constexpr basic_buffer(basic_buffer && rhs) noexcept
      : basic_memory_region<Ptr>(rhs),
        m_owns_buffer(rhs.m_owns_buffer)
    {
      rhs.m_owns_buffer = false;
      rhs.reset();
    }

    /// Construct a buffer, allocating it.
    J_ALWAYS_INLINE constexpr basic_buffer(detail::allocate_tag, u32_t size) noexcept
      : basic_memory_region<Ptr>(size ? j::allocate_zero(size) : nullptr, size),
        m_owns_buffer(size)
    { }

    /// Construct a buffer that owns its contents.
    J_ALWAYS_INLINE constexpr basic_buffer(detail::take_ownership_tag, char * J_NOT_NULL data, u32_t size) noexcept
      : basic_memory_region<Ptr>(size ? data : nullptr, size),
        m_owns_buffer(size)
    { }

    /// Construct a buffer that does not own its contents.
    J_ALWAYS_INLINE constexpr basic_buffer(detail::no_take_ownership_tag, char * J_NOT_NULL data, u32_t size) noexcept
      : basic_memory_region<Ptr>(data, size),
        m_owns_buffer(false)
    { }

    /// Move-assign to a buffer. Rhs will be left empty.
    constexpr basic_buffer & operator=(basic_buffer && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        reset();
        basic_memory_region<Ptr>::operator=(rhs);
        m_owns_buffer = rhs.m_owns_buffer;
        rhs.m_owns_buffer = false;
        rhs.reset();
      }
      return *this;
    }
    J_INLINE_GETTER bool owns_buffer() const noexcept       { return m_owns_buffer; }

    J_ALWAYS_INLINE ~basic_buffer()                         { reset(); }

    J_ALWAYS_INLINE void reset() noexcept {
      if (m_owns_buffer) {
        ::j::free(const_cast<char*>(basic_memory_region<Ptr>::begin()));
      }
      basic_memory_region<Ptr>::reset();
    }
  private:
    bool m_owns_buffer = false;
  };

  class buffer : public detail::basic_buffer_base<char *> {
  public:
    using basic_buffer_base::basic_buffer_base;
    using basic_buffer_base::operator=;

    void reallocate(u32_t new_size) {
      J_ASSUME(new_size > 0U);
      set(basic_memory_region<char *>(::j::reallocate(begin(), new_size), new_size));
    }
  };

  class const_buffer : public detail::basic_buffer_base<const char *> {
  public:
    using basic_buffer_base::basic_buffer_base;
    using basic_buffer_base::operator=;
  };
}
