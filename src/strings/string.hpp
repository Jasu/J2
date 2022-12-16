#pragma once

#include "strings/string_view.hpp"
#include "hzd/mem.hpp"

namespace j::strings {
  enum not_a_string_tag_t { not_a_string_tag };

  class string final {
  public:
    using nicely_copyable_tag_t J_NO_DEBUG_TYPE = void;
    using zero_initializable_tag_t J_NO_DEBUG_TYPE = void;

    J_A(ND) inline static constexpr i32_t npos = -1;

    /// Construct an empty string.
    J_A(AI,ND) inline string() noexcept = default;

    /// Construct an uninitialized string.
    J_A(ND) inline explicit string(i32_t sz) noexcept
      : m_data((u64_t)sz << 1U)
    {
      if (sz >= 7) {
        alloc(sz);
      }
    }

    /// Construct an uninitialized string.
    J_A(AI,ND) inline explicit constexpr string(i32_t sz) noexcept __attribute__((enable_if(sz < 7, "Small string")))
      : m_data(sz << 1U)
    {
    }

    /// Construct a string filled with ch.
    J_A(LEAF) string(i32_t sz, char ch) noexcept;

    /// Construct a string as a copy of a character buffer.
    J_A(LEAF) string(const char * J_NOT_NULL begin, i32_t size) noexcept;

    /// Construct a string as a copy of a character buffer.
    J_A(AI,ND,HIDDEN) inline string(const char * J_NOT_NULL begin, const char * J_NOT_NULL end) noexcept
      : string(begin, end - begin)
    {
    }

    /// Copy-construct a string from a j::strings::const_string_view
    J_A(LEAF) string(const_string_view sv) noexcept;

    J_A(LEAF) string(const char * J_NOT_NULL const c_string) noexcept;

    J_A(AI,ND) inline string(const char * J_NOT_NULL const c_string) noexcept __attribute__((enable_if(__builtin_object_size(c_string, 0) > 7 && __builtin_object_size(c_string, 0) < 0x8000, "Known size")))
    {
      ::j::memcpy(alloc(__builtin_object_size(c_string, 0) - 1),
                  c_string,
                  __builtin_object_size(c_string, 0));
    }

    J_A(AI,ND) inline string(const char * J_NOT_NULL const c_string) noexcept __attribute__((enable_if(__builtin_object_size(c_string, 0) > 0 && __builtin_object_size(c_string, 0) < 8, "Small string")))
      : m_data((__builtin_object_size(c_string, 0) - 1) << 1U)
    {
      ::j::memcpy(data_stack(), c_string, __builtin_object_size(c_string, 0));
    }

    J_A(LEAF,NODISC) operator string_view() noexcept;

    [[nodiscard]] inline operator const_string_view() const noexcept {
      const u32_t sz = (m_data & 0xFFFFU);
      if (sz & 1) {
        const char *data = (const char*)(m_data >> 16);
        return const_string_view{data, J_LIKELY(sz != 1) ? (i32_t)(sz >> 1) : ((const i32_t*)(data))[-1]};
      }
      return const_string_view{data_stack(), (i32_t)((sz & 0xF) >> 1)};
    }

    /// Construct an empty string holding an u32_t tag value.
    J_A(ND,AI,HIDDEN) inline explicit constexpr string(not_a_string_tag_t, u32_t tag) noexcept
      : m_data((uptr_t)tag << 32U | 0x80UL)
    { }

    /// Move constructor.
    J_A(AI,ND) inline constexpr string(string && rhs) noexcept
      : m_data(rhs.m_data)
    { rhs.m_data = 0; }

    /// Copy constructor.
    J_A(LEAF) string(const string & rhs) noexcept ;

    inline ~string() {
      clear();
    }

    /// Move assign.
    inline string & operator=(string && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        clear();
        m_data = rhs.m_data;
        rhs.m_data = 0;
      }
      return *this;
    }

    /// Copy assign from a string_view.
    J_A(LEAF) string & operator=(const const_string_view & rhs) noexcept;

    J_A(LEAF) string & operator=(const string & rhs) noexcept;

    J_A(AI,HIDDEN) inline string & operator=(const char * J_NOT_NULL s) noexcept {
      return operator=(const_string_view(s));
    }

    J_A(RNN,NODISC,RNN) inline char * data() noexcept {
      return (m_data & 1U) ? data_heap() : data_stack();
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) char * data_heap() noexcept J_RETURNS_ALIGNED(8)
    { return reinterpret_cast<char*>(m_data >> 16U); }

    J_A(AI,ND,HIDDEN,NODISC,RNN) const char * data_heap() const noexcept J_RETURNS_ALIGNED(8)
    { return reinterpret_cast<const char*>(m_data >> 16U); }

    J_A(AI,ND,HIDDEN,NODISC,RNN) char * data_stack() noexcept {
#if J_IS_LITTLE_ENDIAN
      return reinterpret_cast<char*>(&m_data) + 1;
#else
      return reinterpret_cast<char*>(&m_data);
#endif
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) const char * data_stack() const noexcept {
#if J_IS_LITTLE_ENDIAN
      return reinterpret_cast<const char*>(&m_data) + 1;
#else
      return reinterpret_cast<const char*>(&m_data);
#endif
    }

    J_A(AI,ND,HIDDEN,NODISC,RNN) const char * data() const noexcept {
      return const_cast<string*>(this)->data();
    }

    J_A(AI,ND,NODISC,HIDDEN,RNN) char * begin() noexcept { return data(); }

    J_A(AI,ND,NODISC,HIDDEN,RNN) const char * begin() const noexcept
    { return const_cast<string*>(this)->data(); }

    [[nodiscard]] J_RETURNS_NONNULL char * end() noexcept
    { return data() + size(); }

    [[nodiscard]] J_RETURNS_NONNULL const char * end() const noexcept {
      return const_cast<string*>(this)->end();
    }

    [[nodiscard]] i32_t size() const noexcept {
      const u32_t sz = (m_data & 0xFFFFU);
      if (sz & 1) {
        return J_LIKELY(sz != 1) ? sz >> 1: ((const i32_t*)(m_data >> 16))[-1];
      } else {
        return (sz & 0xF) >> 1;
      }
    }

    [[nodiscard]] i32_t size_heap() const noexcept {
      const u16_t sz = (u16_t)m_data >> 1U;
      return J_LIKELY(sz) ? sz : reinterpret_cast<const i32_t*>(m_data >> 16)[-1];
    }

    J_A(AI,NODISC,HIDDEN,ND) inline bool empty() const noexcept
    { return !(m_data & 0x0FU); }

    J_A(AI,NODISC,HIDDEN,ND) inline explicit operator bool() const noexcept
    { return m_data & 0x0FU; }

    J_A(AI,NODISC,HIDDEN,ND) inline bool operator!() const noexcept
    { return !(m_data & 0x0FU); }

    J_A(AI,NODISC) inline const char & operator[](i32_t i) const noexcept {
      return data()[i];
    }

    J_A(AI,NODISC) inline char & operator[](i32_t i) noexcept {
      return data()[i];
    }

    inline void clear() noexcept {
      if (m_data & 1U) {
        ::j::free((void*)((m_data >> 16) & ~15));
      }
      m_data = 0;
    }


    [[nodiscard]] const_string_view slice(i32_t start, i32_t len) const noexcept {
      return const_string_view(data() + start, len);
    }

    [[nodiscard]] const_string_view without_prefix(i32_t len) const noexcept {
      return const_string_view(data() + len, size() - len);
    }

    [[nodiscard]] const_string_view without_suffix(i32_t len) const noexcept {
      return const_string_view(data(), size() - len);
    }

    [[nodiscard]] const_string_view suffix(i32_t len) const noexcept {
      return const_string_view(end() - len, len);
    }

    [[nodiscard]] const_string_view prefix(i32_t len) const noexcept {
      return const_string_view(data(), len);
    }

    J_A(LEAF) void erase(i32_t start, i32_t len) noexcept;

    J_A(LEAF) void insert(i32_t at, const_string_view str) noexcept;

    J_A(LEAF) void append(const char * J_NOT_NULL src, i32_t sz) noexcept;

    J_A(AI,ND,HIDDEN) inline void append(const char * J_NOT_NULL src, const char * J_NOT_NULL end) noexcept {
      append(src, end - src);
    }

    J_A(LEAF) void push_back(const char ch) noexcept;

    J_A(LEAF) string & operator+=(const_string_view sv) noexcept;

    J_A(LEAF) string & operator+=(const char * J_NOT_NULL rhs) noexcept;

    J_A(AI,NODISC,HIDDEN,ND) inline bool is_not_a_string() const noexcept
    { return (u8_t)m_data == 0x80; }

    J_A(AI,NODISC,HIDDEN,ND) inline u32_t not_a_string_value() const noexcept
    { return m_data >> 32; }

    uptr_t m_data = 0ULL;
  private:
    J_A(LEAF,RNN) char * alloc(i32_t size) noexcept J_RETURNS_ALIGNED(8);
    J_A(LEAF,RNN) char * realloc(i32_t size) noexcept J_RETURNS_ALIGNED(8);
    J_A(LEAF,RNN) char * realloc_no_keep(i32_t size) noexcept J_RETURNS_ALIGNED(8);
    J_A(LEAF,RNN) char * J_INTERNAL_LINKAGE post_alloc(void * J_NOT_NULL ptr, i32_t size) noexcept;
  };

  template<typename T>
  concept AnActualString = __is_same(T, string);

  template<typename T>
  concept AnActualStringView = __is_same(T, const_string_view) || __is_same(T, string_view);

  template<AnActualString Str>
  inline bool operator==(const Str & lhs, const Str & rhs) noexcept {
    const u16_t lhs_tag = lhs.m_data;
    if (lhs_tag != (u16_t)rhs.m_data) {
      return false;
    }
    if (lhs_tag & 1U) {
      return !j::strcmp(lhs.data_heap(), rhs.data_heap());
    } else {
      return lhs.m_data == rhs.m_data;
    }
  }

  template<AnActualString Str>
  J_A(AI,NODISC) inline bool operator==(const char * J_NOT_NULL lhs, const Str & rhs) noexcept {
    return !j::strcmp(lhs, rhs.data());
  }

  template<AnActualString Str>
  J_A(AI,NODISC) inline bool operator==(const Str & lhs, const char * J_NOT_NULL rhs) noexcept {
    return !j::strcmp(rhs, lhs.data());
  }

  template<AnActualString Str, AnActualStringView Sv>
  [[nodiscard]] inline bool operator==(const Str & lhs, Sv rhs) noexcept {
    const auto sz = rhs.size();
    return sz == lhs.size() && !j::memcmp(rhs.data(), lhs.data(), sz);
  }

  template<AnActualString Str, AnActualStringView Sv>
  [[nodiscard]] inline bool operator==(Sv rhs, const Str & lhs) noexcept {
    const auto sz = rhs.size();
    return sz == lhs.size() && !j::memcmp(rhs.data(), lhs.data(), sz);
  }

  J_A(LEAF,NODISC) string operator+(const const_string_view & lhs, const const_string_view & rhs) noexcept;
}
