#pragma once

#include "hzd/concepts.hpp"

namespace j {
  J_ALWAYS_INLINE_NO_DEBUG_FLATTEN void prefetch(void * ptr) noexcept
  { __builtin_prefetch(ptr); }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T align_up(T value, sz_t alignment) noexcept {
    return __builtin_align_up(value, alignment);
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T align_down(T value, sz_t alignment) noexcept {
    return __builtin_align_down(value, alignment);
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr bool is_aligned(T value, sz_t alignment) noexcept {
    return __builtin_is_aligned(value, alignment);
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T page_align_up(T value) noexcept {
    return __builtin_align_up(value, J_PAGE_SIZE);
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T page_align_down(T value) noexcept {
    return __builtin_align_down(value, J_PAGE_SIZE);
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr bool is_page_aligned(T value) noexcept {
    return __builtin_is_aligned(value, J_PAGE_SIZE);
  }

  template<typename... Ts>
  J_NO_DEBUG inline constexpr sz_t max_size_v = ::j::max(sizeof(Ts)...);

  /// Calls aligned_alloc, throwing if allocation fails.
  J_A(LEAF,RNN,NODISC,MALLOC) void * allocate_aligned(u32_t alignment, u32_t size) noexcept;

  /// Calls malloc, throwing if allocation fails.
  J_A(AI,FLATTEN,RNN,NODISC,MALLOC,ALIGN(16),ND) inline void * allocate(u32_t size) noexcept {
    return __builtin_malloc(size);
  }

  template<typename T>
  J_A(AI,FLATTEN,RNN,NODISC,MALLOC,ALIGN(16),ND) inline T * allocate(u32_t count) noexcept {
    return (T*)__builtin_malloc(count * sizeof(T));
  }

  /// Calls calloc, throwing if allocation fails.
  J_A(AI,FLATTEN,RNN,NODISC,MALLOC,ALIGN(16),ND) inline void * allocate_zero(u32_t size) noexcept   {
    return __builtin_calloc(1, size);
  }

  /// Calls realloc, throwing if allocation fails.
  J_A(AI,FLATTEN,RNN,NODISC,ALIGN(16),ND) inline void * reallocate(void * J_A(ALIGN_VAR(16)) previous, u32_t size) noexcept {
    return __builtin_realloc(previous, size);
  }

  template<typename T>
  J_A(AI,FLATTEN,RNN,NODISC,ALIGN(16),ND) inline T * reallocate(T * J_A(ALIGN_VAR(16)) previous, u32_t count) noexcept {
    return (T*)__builtin_realloc(previous, count * sizeof(T));
  }

  /// Calls free().
  J_A(AI,FLATTEN,ND) inline void free(void * J_A(NOESC,ALIGN_VAR(16)) ptr) noexcept {
    __builtin_free(ptr);
  }

  template<typename A>
  concept Allocator = requires(A & allocator, u32_t size) {
    { allocator.allocate(size) } -> Pointer;
  };

  struct malloc_allocator final {
    inline malloc_allocator() noexcept = default;

    J_A(AI,FLATTEN,RNN,NODISC,ALIGN(16),MALLOC,ND) inline void * allocate(u32_t sz) const {
      return __builtin_malloc(sz);
    }
  };

  extern const malloc_allocator malloc_allocator_v J_A(NODESTROY,ND);

  /// Copy non-overlapping memory, returning the beginning of the target region.
  ///
  /// \return `to`
  J_A(AI,FLATTEN,ND,HIDDEN,RNN) inline void* memcpy(void * J_NOT_NULL J_RESTRICT to,
                                                    const void * J_NOT_NULL J_RESTRICT from,
                                                    sz_t size) noexcept {
    return __builtin_memcpy(to, from, size);
  }

  /// Copy non-overlapping memory, returning a pointer to the end of the written memory.
  ///
  /// \return `to + size`
  J_A(AI,FLATTEN,ND,HIDDEN,RNN,NODISC) inline void* mempcpy(void * J_NOT_NULL J_RESTRICT to, const void * J_NOT_NULL J_RESTRICT from, sz_t size) noexcept {
    return __builtin_mempcpy(to, from, size);
  }

  J_A(AI,FLATTEN,ND,HIDDEN,RNN) inline void * memmove(void * J_NOT_NULL to, const void * J_NOT_NULL from, sz_t size) noexcept {
    return __builtin_memmove(to, from, size);
  }

  J_A(AI,FLATTEN,ND,HIDDEN,RNN) inline void* memset(void * J_NOT_NULL target, char ch, sz_t size) noexcept {
    return __builtin_memset(target, ch, size);
  }

  J_A(AI,FLATTEN,ND,HIDDEN) inline void memzero(void * J_NOT_NULL target, sz_t size) noexcept {
    __builtin_bzero(target, size);
  }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"

[[nodiscard]] void* operator new(sz_t);
[[nodiscard]] void* operator new[](sz_t);
void operator delete(void*) noexcept;
void operator delete[](void*) noexcept;
void operator delete(void*, sz_t) noexcept;
void operator delete[](void*, sz_t) noexcept;
[[nodiscard]] void* operator new(sz_t, void* p) noexcept;
[[nodiscard]] void* operator new[](sz_t, void* p) noexcept;

void operator delete  (void*, void*) noexcept;
void operator delete[](void*, void*) noexcept;

#pragma clang diagnostic pop

#define J_ALLOCA(SZ) __builtin_alloca(SZ)
#define J_ALLOCA_NEW(TYPE) ::new (__builtin_alloca(sizeof(TYPE))) TYPE
#define J_ALLOCA_NEW_PADDING(TYPE, PADDING) ::new (__builtin_alloca(sizeof(TYPE) + PADDING)) TYPE

#define J_MEMCPY_INLINE(...) __builtin_memcpy_inline(__VA_ARGS__)
