#pragma once
/// \file (Hap)hazard Template Library sized types
///
/// Like cstdint and cstddef.

/// int8_t
using i8_t   = __INT8_TYPE__;
/// int16_t
using i16_t  = __INT16_TYPE__;
/// int32_t
using i32_t  = __INT32_TYPE__;
/// int64_t
using i64_t  = __INT64_TYPE__;

/// uint8_t
using u8_t   = __UINT8_TYPE__;
/// uint16_t
using u16_t  = __UINT16_TYPE__;
/// uint32_t
using u32_t  = __UINT32_TYPE__;
/// uint64_t
using u64_t  = __UINT64_TYPE__;

/// intptr_t
using iptr_t = __INTPTR_TYPE__;
/// uintptr_t
using uptr_t = __UINTPTR_TYPE__;

/// size_t
using sz_t   = __SIZE_TYPE__;
/// ptrdiff_t
using ssz_t  = __PTRDIFF_TYPE__;

#define I8_MAX  __INT8_MAX__
#define I16_MAX __INT16_MAX__
#define I32_MAX __INT32_MAX__
#define I64_MAX __INT64_MAX__

#define I8_MIN  (-128)
#define I16_MIN (-32768)
#define I32_MIN (-2147483647-1)
#define I64_MIN (-9223372036854775807L)

#define U8_MAX  __UINT8_MAX__
#define U16_MAX __UINT16_MAX__
#define U32_MAX __UINT32_MAX__
#define U64_MAX __UINT64_MAX__
#define UPTR_MAX __UINTPTR_MAX__

#define SZ_MAX __SIZE_MAX__

#ifdef __SIZEOF_INT128__
using i128_t = __int128;
using u128_t = unsigned __int128;
#endif

/// std::nullptr_t
using null_t = decltype(nullptr);
