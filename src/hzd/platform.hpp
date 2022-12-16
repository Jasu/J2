#pragma once

#ifdef __LITTLE_ENDIAN__
#  define J_IS_LITTLE_ENDIAN 1
#  define J_IS_BIG_ENDIAN 0
#else
#  define J_IS_LITTLE_ENDIAN 0
#  define J_IS_BIG_ENDIAN 1
#endif

#define J_REQUIRE_LITTLE_ENDIAN() static_assert(J_IS_LITTLE_ENDIAN == 1)

#ifdef __x86_64__
#  define J_IS_AMD64 1
#else
#  define J_IS_AMD64 0
#endif
