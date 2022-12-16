#include "strings/string.hpp"
#include "exceptions/assert_lite.hpp"
#include "logging/global.hpp"
#include "strings/string_view.hpp"
#include "hzd/string.hpp"
#include "hzd/mem.hpp"

namespace j::strings {
  namespace {
    void fill(void * J_NOT_NULL dst, u32_t sz) noexcept {
      if (J_UNLIKELY(sz >= 0x8000U)) {
        sz += 8;
      }
      char * dst_ = (char*)dst + sz;
      for (u32_t i = (u32_t)sz & 7U; i & 7U; ++i, ++dst_) {
        *dst_ = 0;
      }
    }

    constexpr u32_t align_size(u32_t sz) noexcept {
      return align_up(sz, 8);
    }

    i32_t alloc_size(i32_t sz) noexcept {
      J_ASSUME(sz >= 7);
      return J_LIKELY(sz < 0x8000) ? align_size(sz + 1) : align_size(sz + 9);
    }

    uptr_t make_data_heap(u32_t sz, void * J_NOT_NULL data) noexcept {
      return 1ULL | (J_LIKELY(sz < 0x8000U) ? ((uptr_t)sz << 1) : 0UL) | (reinterpret_cast<uptr_t>(data) << 16);
    }

    uptr_t make_data_small_string(i32_t sz, const char * J_NOT_NULL src) noexcept {
      J_ASSUME(sz < 7);
      uptr_t data = sz << 1;
      const u8_t * src_ = (const u8_t*)src;
      for (i32_t i = 0, shift = 8; i != sz; ++i, shift += 8, ++src_) {
        data |= (uptr_t)*src_ << shift;
      }
      return data;
    }
  }

  string::string(const char * J_NOT_NULL const c_string) noexcept
    : string(c_string, ::j::strlen(c_string))
  { }

  [[nodiscard]] string operator+(const const_string_view & lhs, const const_string_view & rhs) noexcept {
    const u32_t lhs_sz = lhs.size(), rhs_sz = rhs.size();
    string result(lhs_sz + rhs_sz);
    char * dst = result.data();
    ::j::memcpy(dst, lhs.data(), lhs_sz);
    ::j::memcpy(dst + lhs_sz, rhs.data(), rhs_sz);
    *(dst + lhs_sz + rhs_sz) = 0;
    return result;
  }

  void string::append(const char * J_NOT_NULL src, i32_t sz) noexcept {
    if (J_UNLIKELY(sz == 0)) {
      return;
    }
    const i32_t own_sz = size();
    const char * d = data();
    char * ptr = realloc(sz + own_sz);
    if (J_UNLIKELY(src >= d && src < d + own_sz)) {
      src = ptr + (src - d);
    }
    ::j::memcpy(ptr + own_sz, src, sz);
    *(ptr + own_sz + sz) = 0U;
  }

  J_A(LEAF) void string::push_back(const char ch) noexcept {
    append(&ch, 1U);
  }

  string & string::operator=(const const_string_view & rhs) noexcept {
    const i32_t sz = rhs.size();
    if (sz < 7) {
      clear();
      m_data = sz << 1U;
      ::j::memcpy(data_stack(), rhs.data(), sz);
    } else {
      auto ptr = realloc_no_keep(sz);
      ::j::memcpy(ptr, rhs.data(), sz);
      ptr[sz] = 0U;
      J_ASSUME(m_data & 1);
    }
    return *this;

  }

  J_RETURNS_NONNULL char * J_INTERNAL_LINKAGE string::post_alloc(void * J_NOT_NULL ptr, i32_t size) noexcept {
    fill(ptr, size);
    u32_t * ptr_ = (u32_t*)ptr;
    if (J_UNLIKELY(size >= 0x8000)) {
      *++ptr_ = size;
      ++ptr_;
    }
    m_data = make_data_heap(size, ptr_);
    return (char*)ptr_;
  }

  J_RETURNS_NONNULL char * string::alloc(i32_t size) noexcept {
    return post_alloc(::j::allocate(alloc_size(size)), size);
  }

  J_RETURNS_NONNULL char * string::realloc_no_keep(i32_t sz) noexcept {
    i32_t old_size = size();
    if (old_size < 7) {
      return alloc(sz);
    }

    char * ptr = data();
    if (J_UNLIKELY(old_size >= 0x8000)) {
      ptr -= 8U;
    }

    const i32_t alloc_sz = alloc_size(sz);
    if (alloc_sz != alloc_size(old_size)) {
      ptr = (char*)::j::reallocate(ptr, alloc_sz);
    }
    return post_alloc(ptr, sz);
  }

  J_RETURNS_NONNULL char * string::realloc(i32_t sz) noexcept {
    const i32_t old_size = size();
    if (sz < 7) {
      if (old_size < 7) {
        J_REQUIRE_LITTLE_ENDIAN();
        auto d = m_data + (((uptr_t)sz - (uptr_t)old_size) << 1);
        m_data = d & (UPTR_MAX >> (8 * (6 - sz)));
      } else {
        char * d = data();
        m_data = make_data_small_string(sz, d);
        ::j::free(J_UNLIKELY(old_size >= 0x8000) ? d - 8 : d);
      }
      return data_stack();
    }

    const i32_t alloc_sz = alloc_size(sz);
    void * ptr = nullptr;
    if (old_size < 7) {
      ptr = ::j::allocate(alloc_sz);
      ::j::memcpy(J_UNLIKELY(sz >= 0x8000) ? (char*)ptr + 8U : (char*)ptr,
                  data_stack(),
                  old_size + 1U);
    } else {
      ptr = data();
      if (J_UNLIKELY(old_size >= 0x8000)) {
        ptr = (char*)ptr - 8U;
      }
      if (alloc_sz != alloc_size(old_size)) {
        ptr = ::j::reallocate(ptr, alloc_sz);
      }
    }
    return post_alloc(ptr, sz);
  }

  void string::erase(i32_t start, i32_t len) noexcept {
    const i32_t sz = size();
    J_ASSERT(start + len <= sz, "Out of range.");
    if (!len) {
      return;
    }
    char * d = data();
    ::j::memmove(d + start, d + start + len, sz - start - len + 1);
    realloc(sz - len);
  }

  void string::insert(i32_t at, const_string_view str) noexcept {
    const i32_t sz = size(), added_sz = str.size();
    J_ASSERT(at <= sz, "Out of range.");
    if (!added_sz) {
      return;
    }
    char * ptr = realloc(alloc_size(sz + added_sz));
    ::j::memmove(ptr + at + added_sz, ptr + at, sz - at);
    ::j::memcpy(ptr + at, str.data(), added_sz);
    *(ptr + at + added_sz) = 0U;
  }

  string::string(const char * J_NOT_NULL begin, i32_t size) noexcept {
    if (size < 7) {
      m_data = size << 1U;
      ::j::memcpy(data_stack(), begin, size);
    } else {
      auto ptr = alloc(size);
      ::j::memcpy(ptr, begin, size);
      ptr[size] = 0;
    }
  }
  string::string(i32_t sz, char ch) noexcept {
    if (sz < 7) {
      m_data = sz << 1U;
      ::j::memset(data_stack(), ch, sz);
    } else {
      auto ptr = alloc(sz);
      ::j::memset(ptr, ch, sz);
      ptr[sz] = 0U;
    }
  }

  string::string(const_string_view sv) noexcept
    : string(sv.data(), sv.size())
  { }

  string::string(const string & rhs) noexcept {
    const i32_t sz = rhs.size();
    if (sz >= 7) {
      ::j::memcpy(alloc(sz), rhs.data_heap(), sz + 1);
    } else {
      m_data = rhs.m_data;
    }
  }
  string::operator string_view() noexcept {
    return string_view{data(), size()};
  }

  string & string::operator=(const string & rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      operator=(const_string_view(rhs));
    }
    return *this;
  }

  string & string::operator+=(const_string_view sv) noexcept{
    if (sv) {
      append(sv.data(), sv.size());
    }
    return *this;
  }

  string & string::operator+=(const char * J_NOT_NULL rhs) noexcept {
    append(rhs, ::j::strlen(rhs));
    return *this;
  }

  [[nodiscard]] bool operator<(const_string_view lhs, const_string_view rhs) noexcept {
    const i32_t prefix_diff = ::j::memcmp(lhs.m_data, rhs.m_data, ::j::min(lhs.m_size, rhs.m_size));
    return prefix_diff ? prefix_diff < 0 : lhs.m_size < rhs.m_size;
  }
}
