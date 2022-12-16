#pragma once

#include "hzd/types.hpp"
#include "hzd/platform.hpp"

#ifdef __GNUC__
# define J_FUNC __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
# define J_FUNC __FUNCSIG__
#else
# define J_FUNC __FUNC__
#endif

/// Get the size of ARRAY in objects.
#define J_ARRAY_SIZE(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#define J_PTR_SZ 8

/// Indicate that the position is not reachable.
///
/// In practice, this means that executing J_UNREACHABLE() is undefined behavior.
/// This is useful in e.g. switch-cases to indicate that some cases are not
/// possible, while not causing warnings about non-exhaustiveness.
#define J_UNREACHABLE() __builtin_unreachable()

/// Indicate that the expression CONDITION likely evaluates to false.
///
/// The result is the result of CONDITION.
#define J_UNLIKELY(CONDITION) __builtin_expect((bool)(CONDITION), false)
/// Indicate that the expression CONDITION likely evaluates to true.
///
/// The result is the result of CONDITION.
#define J_LIKELY(CONDITION) __builtin_expect((bool)(CONDITION), true)

#define J_U128_HEX(A, B) (((u128_t)(0x##A##ULL)<<64)|(u128_t)(0x##B##ULL))
#define J_NOT_NULL __attribute__((nonnull))

#define J_NOESCAPE __attribute__((noescape))
#define J_NOT_NULL_NOESCAPE __attribute__((nonnull,noescape))

/// Evaluates to true when evaluating consexprs.
#define J_IS_CONSTEVAL __builtin_is_constant_evaluated

#define J_NOT_NULL_NOESCAPE __attribute__((nonnull,noescape))
/// Indicate that callee is responsible for deleting the object.
///
/// Normally, classes with non-trivial dtors or copy/move ctors are "owned" by the caller,
/// and deleted by the caller. This is a problem with e.g. passing unique or shared pointers.
#define J_TRIVIAL_ABI __attribute__((trivial_abi))

/// Get the byte offset of MEMBER in TYPE.
#define J_OFFSET_OF(TYPE, MEMBER) __builtin_offsetof(TYPE, MEMBER)

/// Get the byte offset of MEMBER in TYPE.
#define J_OFFSET_OF_NON_STANDARD(TYPE, MEMBER) \
  __builtin_bit_cast(ssz_t, &((TYPE*)nullptr)->MEMBER)

/// Get the address of containing struct/class of member pointed by PTR.
#define J_CONTAINER_OF(PTR, CONTAINER, MEMBER)                                    \
  reinterpret_cast<CONTAINER * J_NOT_NULL>(                                       \
    const_cast<char * J_NOT_NULL>(reinterpret_cast<const char * J_NOT_NULL>(PTR)) \
  - J_OFFSET_OF_NON_STANDARD(CONTAINER, MEMBER))

#ifndef J_CONFIG_DISABLE_VISIBILITY_HIDDEN
#define J_HIDDEN_ATTRIBUTE , __visibility__("hidden")
#define J_HIDDEN __attribute__((__visibility__("hidden")))
#define J_TYPE_HIDDEN __attribute__((__type_visibility__("hidden")))
#else
#define J_HIDDEN_ATTRIBUTE
#define J_HIDDEN
#define J_TYPE_HIDDEN
#endif

/// Mark a function to be inlined also in debug builds.
#define J_ALWAYS_INLINE __attribute__((__always_inline__ J_HIDDEN_ATTRIBUTE)) inline

/// Mark a function to be inlined also in debug builds.
#define J_ALWAYS_INLINE_NONNULL __attribute__((__always_inline__ J_HIDDEN_ATTRIBUTE, returns_nonnull)) inline

/// Make compiler inline all calls within this function.
#define J_FLATTEN __attribute__((flatten)) inline

/// Mark a function to be inlined also in debug builds.
#define J_ALWAYS_INLINE_FLATTEN __attribute__((__always_inline__, flatten J_HIDDEN_ATTRIBUTE)) inline

#define J_NO_EXPLICIT __attribute__((exclude_from_explicit_instantiation))

/// Mark a function as an always-inlined nodiscard function.
#define J_INLINE_GETTER [[nodiscard]] __attribute__((__always_inline__ J_HIDDEN_ATTRIBUTE)) inline

/// Mark a function as an always-inlined nodiscard functionreturning non-null.
#define J_INLINE_GETTER_NONNULL [[nodiscard]] __attribute__((__always_inline__ J_HIDDEN_ATTRIBUTE, returns_nonnull)) inline

/// Mark a function as an always-inlined nodiscard function.
#define J_INLINE_GETTER_FLATTEN [[nodiscard]] __attribute__((__always_inline__, flatten J_HIDDEN_ATTRIBUTE)) inline

/// Mark a function to never be inlined.
///
/// This is especially useful for initialization / finalization code and for wrappers around throw,
/// since throw generates a large amount of code (or stack unwinding context)
#define J_NO_INLINE __attribute__((noinline))

#define J_INTERNAL_LINKAGE __attribute__((internal_linkage))

#ifndef J_CONFIG_DISABLE_NO_DEBUG

#define J_NO_DEBUG_ATTR , nodebug

#define J_NO_DEBUG __attribute__((nodebug))

/// Attribute that can be added to types for which no debug information should
/// be emitted.
///
/// Usage:
///     using foo J_NO_DEBUG_TYPE = int;
///     typedef J_NO_DEBUG_TYPE foo int;
#define J_NO_DEBUG_TYPE __attribute__((nodebug))
#else

#define J_NO_DEBUG_ATTR
#define J_NO_DEBUG
#define J_NO_DEBUG_TYPE

#endif


/// Mark a function to be inlined also in debug builds, and omit debug info.
#define J_ALWAYS_INLINE_NO_DEBUG __attribute__((__always_inline__ J_NO_DEBUG_ATTR J_HIDDEN_ATTRIBUTE)) inline

/// Mark a function to be inlined also in debug builds, and omit debug info.
#define J_ALWAYS_INLINE_NO_DEBUG_FLATTEN __attribute__((__always_inline__, flatten J_NO_DEBUG_ATTR J_HIDDEN_ATTRIBUTE)) inline

/// Mark a function as an always-inlined nodiscard function and omit debug info.
#define J_INLINE_GETTER_NO_DEBUG [[nodiscard]] __attribute__((__always_inline__ J_NO_DEBUG_ATTR J_HIDDEN_ATTRIBUTE)) inline

/// Mark a function as an always-inlined, non-null returning nodiscard function and omit debug info.
#define J_INLINE_GETTER_NONNULL_NO_DEBUG [[nodiscard]] __attribute__((__always_inline__ J_NO_DEBUG_ATTR  J_HIDDEN_ATTRIBUTE, returns_nonnull)) inline

/// Mark a function as an always-inlined nodiscard function and omit debug info.
#define J_INLINE_GETTER_NO_DEBUG_FLATTEN [[nodiscard]] __attribute__((__always_inline__ J_NO_DEBUG_ATTR, flatten J_HIDDEN_ATTRIBUTE)) inline

#define J_RETURNS_MALLOC __attribute__((__malloc__))
#define J_RETURNS_NONNULL __attribute__((returns_nonnull))

#define J_NO_STACK_PROTECTOR __attribute__((no_stack_protector, no_sanitize("safe-stack")))

#define J_RESTRICT __restrict__

#define J_PACKED __attribute__((packed))

#define J_RETURNS_ALIGNED(...) __attribute__((assume_aligned (__VA_ARGS__)))
#define J_ASSUME_ALIGNED(...) __builtin_assume_aligned(__VA_ARGS__)
#define J_ASSUME_ALIGNED_16(...) __builtin_assume_aligned(__VA_ARGS__, 16)
#define J_ASSUME_ALIGNED_8(...) __builtin_assume_aligned(__VA_ARGS__, 8)
#define J_ASSUME_ALIGNED_4(...) __builtin_assume_aligned(__VA_ARGS__, 4)
#define J_VAR_ALIGNED(...) __attribute__((align_value (__VA_ARGS__)))

#define J_PAGE_SIZE 4096U
#define J_PAGE_SIZE_MASK 4095U

#define J_RETURNS_PAGE_ALIGNED __attribute__((assume_aligned (J_PAGE_SIZE)))
#define J_ASSUME_PAGE_ALIGNED(...) __builtin_assume_aligned(__VA_ARGS__, J_PAGE_SIZE)
#define J_PAGE_ALIGNED __attribute__((align_value (J_PAGE_SIZE)))

#define J_TLS_GLOBAL_DYNAMIC __attribute__((tls_model("global-dynamic")))
#define J_TLS_LOCAL_DYNAMIC __attribute__((tls_model("local-dynamic")))
#define J_TLS_INITIAL_EXEC __attribute__((tls_model("initial-exec")))
#define J_TLS_LOCAL_EXEC __attribute__((tls_model("local-exec")))

#define J_A_WRAP_GCC(...) __VA_OPT__(__attribute__((__VA_ARGS__)))

#define J_AGCC_ALIGN(A) assume_aligned(A),
#define J_AGCC_PGALIGN assume_aligned (J_PAGE_SIZE),
#define J_AGCC_ALIGN_VAR(A) align_value(A),
#define J_AGCC_NI noinline,
#define J_AGCC_HIDDEN visibility("hidden"),
#define J_AGCC_PURE pure,
#define J_AGCC_CONST const,
#define J_AGCC_HOT hot,
#define J_AGCC_COLD cold,
#define J_AGCC_MALLOC __malloc__,
#define J_AGCC_NOESC noescape,
#define J_AGCC_NORET cold,
#define J_AGCC_MU
#define J_AGCC_NE
#define J_AGCC_RNN
#define J_AGCC_AI
#define J_AGCC_RNN
#define J_AGCC_ND
#define J_AGCC_FLATTEN
#define J_AGCC_NODESTROY
#define J_AGCC_NODISC
#define J_AGCC_LEAF
#define J_AGCC_FLAGS

#define J_ACXX_ALIGN(A)
#define J_ACXX_ALIGN_VAR(A)
#define J_ACXX_PGALIGN
#define J_ACXX_NI
#define J_ACXX_HIDDEN
#define J_ACXX_PURE
#define J_ACXX_CONST
#define J_ACXX_HOT
#define J_ACXX_COLD
#define J_ACXX_MALLOC
#define J_ACXX_NOESC
#define J_ACXX_MU [[maybe_unused]]
#define J_ACXX_NORET [[noreturn]]
#define J_ACXX_NE [[clang::exclude_from_explicit_instantiation]]
#define J_ACXX_RNN [[gnu::returns_nonnull]]
#define J_ACXX_AI [[gnu::always_inline]]
#define J_ACXX_RNN [[gnu::returns_nonnull]]
#define J_ACXX_ND [[gnu::nodebug]]
#define J_ACXX_FLATTEN [[gnu::flatten]]
#define J_ACXX_NODESTROY [[clang::no_destroy]]
#define J_ACXX_NODISC [[nodiscard]]
#define J_ACXX_LEAF [[gnu::leaf]]
#define J_ACXX_FLAGS [[clang::flag_enum]]

#define J_A(...) J_PREFIX(J_ACXX_, __VA_ARGS__) J_PREFIX_FILTER(J_AGCC_, J_A_WRAP_GCC, __VA_ARGS__)
#define J_AA(...) J_PREFIX(J_AA_, __VA_ARGS__) J_PREFIX_FILTER(J_AAGCC_, J_A_WRAP_GCC, __VA_ARGS__)
#define J_AV(...) J_PREFIX(J_AV_, __VA_ARGS__) J_PREFIX_FILTER(J_AVGCC_, J_A_WRAP_GCC, __VA_ARGS__)
#define J_AT(...) J_PREFIX(J_AT_, __VA_ARGS__) J_PREFIX_FILTER(J_ATGCC_, J_A_WRAP_GCC, __VA_ARGS__)

#define J_AA_NOALIAS __restrict__
#define J_AA_MU [[maybe_unused]]
#define J_AA_NOESC
#define J_AA_NN

#define J_AAGCC_NOALIAS
#define J_AAGCC_MU
#define J_AAGCC_NN nonnull
#define J_AAGCC_NOESC noescape

#define J_ATGCC_HIDDEN type_visibility("hidden"),
#define J_ATGCC_PROT type_visibility("protected"),
#define J_ATGCC_PACK packed
#define J_ATGCC_PREF_NAME(N)

#define J_AT_HIDDEN
#define J_AT_PROT
#define J_AT_PACK
#define J_AT_PREF_NAME(N) [[clang::preferred_name(N)]]

#define J_AVGCC_HIDDEN visibility("hidden"),
#define J_AVGCC_PROTECTED visibility("protected"),
#define J_AVGCC_ND nodebug,
#define J_AVGCC_NOALIAS

#define J_AV_HIDDEN
#define J_AV_PROTECTED
#define J_AV_ND
#define J_AV_NOALIAS __restrict__

#define J_VA_COUNT_IMPL(N9, N8, N7, N6, N5, N4, N3, N2, N1, M, ...) M
#define J_VA_COUNT(...) J_VA_COUNT_IMPL(__VA_ARGS__ __VA_OPT__(,) 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define J_FE0(M)
#define J_FE1(M, A) M(A)
#define J_FE2(M, A, B) M(A) M(B)
#define J_FE3(M, A, B, C) M(A) M(B) M(C)
#define J_FE4(M, A, B, C, D) M(A) M(B) M(C) M(D)
#define J_FE5(M, A, B, C, D, E) M(A) M(B) M(C) M(D) M(E)
#define J_FE6(M, A, B, C, D, E, F) M(A) M(B) M(C) M(D) M(E) M(F)
#define J_FE_IMPL(N6, N5, N4, N3, N2, N1, M, ...) M
#define J_FE(M, ...) J_FE_IMPL(__VA_ARGS__, J_FE6, J_FE5, J_FE4, J_FE3, J_FE2, J_FE1, J_FE0)(M, __VA_ARGS__)

#define J_FEC0(M)
#define J_FEC1(M, A) M(A)
#define J_FEC2(M, A, B) M(A), M(B)
#define J_FEC3(M, A, B, C) M(A), M(B), M(C)
#define J_FEC4(M, A, B, C, D) M(A), M(B), M(C), M(D)
#define J_FEC5(M, A, B, C, D, E) M(A), M(B), M(C), M(D), M(E)
#define J_FEC6(M, A, B, C, D, E, F) M(A), M(B), M(C), M(D), M(E), M(F)
#define J_FEC7(M, A, B, C, D, E, F, G) M(A), M(B), M(C), M(D), M(E), M(F), M(G)
#define J_FEC_IMPL(N7, N6, N5, N4, N3, N2, N1, M, ...) M
#define J_FEC(M, ...) J_FEC_IMPL(__VA_ARGS__, J_FEC7, J_FEC6, J_FEC5, J_FEC4, J_FEC3, J_FEC2, J_FEC1, J_FEC0)(M, __VA_ARGS__)

#define J_FEC20(M)
#define J_FEC21(M, A) M(A)
#define J_FEC22(M, A, B) M(A), M(B)
#define J_FEC23(M, A, B, C) M(A), M(B), M(C)
#define J_FEC24(M, A, B, C, D) M(A), M(B), M(C), M(D)
#define J_FEC25(M, A, B, C, D, E) M(A), M(B), M(C), M(D), M(E)
#define J_FEC26(M, A, B, C, D, E, F) M(A), M(B), M(C), M(D), M(E), M(F)
#define J_FEC27(M, A, B, C, D, E, F, G) M(A), M(B), M(C), M(D), M(E), M(F), M(G)
#define J_FEC2_IMPL(N7, N6, N5, N4, N3, N2, N1, M, ...) M
#define J_FEC_2(M, ...) J_FEC2_IMPL(__VA_ARGS__, J_FEC27, J_FEC26, J_FEC25, J_FEC24, J_FEC23, J_FEC22, J_FEC21, J_FEC20)(M, __VA_ARGS__)

#define J_FE0_POSTARG(M, A)
#define J_FE1_POSTARG(M, A, V)      M(V, A)
#define J_FE2_POSTARG(M, A, V, ...) M(V, A)J_FE1_POSTARG(M, A, __VA_ARGS__)
#define J_FE3_POSTARG(M, A, V, ...) M(V, A)J_FE2_POSTARG(M, A, __VA_ARGS__)
#define J_FE4_POSTARG(M, A, V, ...) M(V, A)J_FE3_POSTARG(M, A, __VA_ARGS__)
#define J_FE5_POSTARG(M, A, V, ...) M(V, A)J_FE4_POSTARG(M, A, __VA_ARGS__)
#define J_FE6_POSTARG(M, A, V, ...) M(V, A)J_FE5_POSTARG(M, A, __VA_ARGS__)
#define J_FE_POSTARG(M, A, ...) J_FE_IMPL(__VA_ARGS__, J_FE6_POSTARG, J_FE5_POSTARG, J_FE4_POSTARG, J_FE3_POSTARG, J_FE2_POSTARG, J_FE1_POSTARG, J_FE0_POSTARG)(M, A, __VA_ARGS__)

#define J_FE_PRE1(M, ARG, A) M(ARG, A)
#define J_FE_PRE2(M, ARG, A, B) M(ARG, A) M(ARG, B)
#define J_FE_PRE3(M, ARG, A, B, C) M(ARG, A) M(ARG, B) M(ARG, C)
#define J_FE_PRE4(M, ARG, A, B, C, D) M(ARG, A) M(ARG, B) M(ARG, C) M(ARG, D)
#define J_FE_PRE5(M, ARG, A, B, C, D, E) M(ARG, A) M(ARG, B) M(ARG, C) M(ARG, D) M(ARG, E)
#define J_FE_PRE6(M, ARG, A, B, C, D, E, F) M(ARG, A) M(ARG, B) M(ARG, C) M(ARG, D) M(ARG, E) M(ARG, F)
#define J_FE_PRE7(M, ARG, A, B, C, D, E, F, G) M(ARG, A) M(ARG, B) M(ARG, C) M(ARG, D) M(ARG, E) M(ARG, F) M(ARG, G)
#define J_FE_PRE_IMPL(A, B, C, D, E, F, G, M, ...) J_FE_PRE ## M
#define J_FE_PREARG(M, ARG, ...) __VA_OPT__(J_FE_PRE_IMPL(__VA_ARGS__,7,6,5,4,3,2,1)(M, ARG, __VA_ARGS__))

#define J_PASS(...) __VA_ARGS__
#define J_FIRST_IMPL(A, ...) A
#define J_FIRST(...) J_FIRST_IMPL(__VA_ARGS__)
#define J_SECOND_IMPL(A, B, ...) B
#define J_SECOND(...) J_SECOND_IMPL(__VA_ARGS__)

#define J_REST(A, ...) __VA_ARGS__

#define J_VA_DEFAULTS(DEFAULT, ...) J_FIRST_IMPL(__VA_OPT__(J_REST,) J_FIRST)(DEFAULT, __VA_ARGS__)

/// Expands a #define, quoting its value as a stirng. E.g. J_STRINGIFY(__LINE__) -> "123"
#define J_STRINGIFY(A) J_STRINGIFY_IMPL(A)
#define J_STRINGIFY_IMPL(A) #A

#define J_CAT_IMPL(A, B) A ## B
#define J_CAT(A, B) J_CAT_IMPL(A, B)
#define J_UNIQUE(N) J_CAT(N, __COUNTER__)
#define J_CAT_ALL_IMPL(A,B,C,D,E,F,G,H,  I,J,K,L,M,N,O,P,   Q,R,S,T,U,V,W,X, ...) \
  A ## B ## C ## D    ##    E ## F ## G ## H    ##    I ## J ## K ## L ## \
  M ## N ## O ## P    ##    Q ## R ## S ## T    ##    U ## V ## W ## X
#define J_CAT_ALL(...) J_CAT_ALL_IMPL(__VA_ARGS__,,,,,,,,,,,,,,,,,,,,,,,)
#define J_B(...) J_CAT_ALL(0b, __VA_ARGS__ U)

#define J_BIN(...) J_CAT_ALL(0b, __VA_ARGS__, U)
#define J_HEX(...) J_CAT_ALL(0x, __VA_ARGS__, U)

#define J_BIT(BIT) (1ULL << (BIT))

#define J_PREFIX_0(P)
#define J_PREFIX_1(P, A) P##A
#define J_PREFIX_2(P, A, B) P##A P##B
#define J_PREFIX_3(P, A, B, C) P##A P##B P##C
#define J_PREFIX_4(P, A, B, C, D) P##A P##B P##C P##D
#define J_PREFIX_5(P, A, B, C, D, E) P##A P##B P##C P##D P##E
#define J_PREFIX_6(P, A, B, C, D, E, F) P##A P##B P##C P##D P##E P##F
#define J_PREFIX_7(P, A, B, C, D, E, F, G) P##A P##B P##C P##D P##E P##F P##G
#define J_PREFIX_8(P, A, B, C, D, E, F, G, H) P##A P##B P##C P##D P##E P##F P##G P##H
#define J_PREFIX_9(P, A, B, C, D, E, F, G, H, I) P##A P##B P##C P##D P##E P##F P##G P##H P##I
#define J_PREFIX_10(P, A, B, C, D, E, F, G, H, I, J) P##A P##B P##C P##D P##E P##F P##G P##H P##I P##J
#define J_PREFIX_IMPL(A, B, C, D, E, F, G, H, I, J, M, ...) J_PREFIX_ ## M
#define J_PREFIX(P, ...) __VA_OPT__(J_PREFIX_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)(P,__VA_ARGS__))

#define J_PREFIX_FILTER_0(P, M)
#define J_PREFIX_FILTER_1(P, M, A) M(P##A)
#define J_PREFIX_FILTER_2(P, M, A, B) M(P##A P##B)
#define J_PREFIX_FILTER_3(P, M, A, B, C) M(P##A P##B P##C)
#define J_PREFIX_FILTER_4(P, M, A, B, C, D) M(P##A P##B P##C P##D)
#define J_PREFIX_FILTER_5(P, M, A, B, C, D, E) M(P##A P##B P##C P##D P##E)
#define J_PREFIX_FILTER_6(P, M, A, B, C, D, E, F) M(P##A P##B P##C P##D P##E P##F)
#define J_PREFIX_FILTER_7(P, M, A, B, C, D, E, F, G) M(P##A P##B P##C P##D P##E P##F P##G)
#define J_PREFIX_FILTER_8(P, M, A, B, C, D, E, F, G, H) M(P##A P##B P##C P##D P##E P##F P##G P##H)
#define J_PREFIX_FILTER_9(P, M, A, B, C, D, E, F, G, H, I) M(P##A P##B P##C P##D P##E P##F P##G P##H P##I)
#define J_PREFIX_FILTER_10(P, M, A, B, C, D, E, F, G, H, I, J) M(P##A P##B P##C P##D P##E P##F P##G P##H P##I P##J)
#define J_PREFIX_FILTER_IMPL(A, B, C, D, E, F, G, H, I, J, M, ...) J_PREFIX_FILTER_ ## M
#define J_PREFIX_FILTER(P, M, ...) J_PREFIX_FILTER_IMPL(__VA_ARGS__ __VA_OPT__(,) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)(P, M __VA_OPT__(,) __VA_ARGS__)


#define J_BOILERPLATE_EQ(CLASS) J_INLINE_GETTER bool operator==(const CLASS &) const noexcept = default;
#define J_BOILERPLATE_EQ_CE(CLASS) J_INLINE_GETTER constexpr bool operator==(const CLASS &) const noexcept = default;

#define J_BOILERPLATE_CTOR(CLASS) CLASS() = default;
#define J_BOILERPLATE_CTOR_ND(CLASS) J_A(ND) CLASS() = default;
#define J_BOILERPLATE_CTOR_NE(CLASS) CLASS() noexcept = default;
#define J_BOILERPLATE_CTOR_CE(CLASS) constexpr CLASS() noexcept = default;
#define J_BOILERPLATE_CTOR_DEL(CLASS) CLASS() = delete;
#define J_BOILERPLATE_CTOR_NE_ND(CLASS) J_A(ND) CLASS() noexcept = default;
#define J_BOILERPLATE_CTOR_CE_ND(CLASS) J_A(ND) constexpr CLASS() noexcept = default;
#define J_BOILERPLATE_CTOR_DEL_ND(CLASS) J_A(ND) CLASS() = delete;

#define J_BOILERPLATE_COPY_CTOR(CLASS) CLASS(const CLASS & rhs) = default;
#define J_BOILERPLATE_COPY_CTOR_ND(CLASS) J_A(ND) CLASS(const CLASS & rhs) = default;
#define J_BOILERPLATE_COPY_CTOR_NE(CLASS) CLASS(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_CTOR_CE(CLASS) constexpr CLASS(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_CTOR_DEL(CLASS) CLASS(const CLASS & rhs) = delete;
#define J_BOILERPLATE_COPY_CTOR_NE_ND(CLASS) J_A(ND) CLASS(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_CTOR_CE_ND(CLASS) J_A(ND) constexpr CLASS(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_CTOR_DEL_ND(CLASS) J_A(ND) CLASS(const CLASS & rhs) = delete;

#define J_BOILERPLATE_COPY_ASSIGN(CLASS) CLASS & operator=(const CLASS & rhs) = default;
#define J_BOILERPLATE_COPY_ASSIGN_NE(CLASS) CLASS & operator=(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_ASSIGN_CE(CLASS) constexpr CLASS & operator=(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_ASSIGN_DEL(CLASS) CLASS & operator=(const CLASS & rhs) = delete;
#define J_BOILERPLATE_COPY_ASSIGN_NE_ND(CLASS) J_A(ND) CLASS & operator=(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_ASSIGN_CE_ND(CLASS) J_A(ND) constexpr CLASS & operator=(const CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_COPY_ASSIGN_DEL_ND(CLASS) J_A(ND) CLASS & operator=(const CLASS & rhs) = delete;

#define J_BOILERPLATE_MOVE_CTOR(CLASS) CLASS(CLASS && rhs) = default;
#define J_BOILERPLATE_MOVE_CTOR_NE(CLASS) CLASS(CLASS && rhs) noexcept = default;
#define J_BOILERPLATE_MOVE_CTOR_CE(CLASS) J_ALWAYS_INLINE constexpr CLASS(CLASS && rhs) noexcept = default;
#define J_BOILERPLATE_MOVE_CTOR_DEL(CLASS) CLASS(CLASS && rhs) = delete;
#define J_BOILERPLATE_MOVE_CTOR_ND(CLASS) J_A(ND) CLASS(CLASS && rhs) = default;
#define J_BOILERPLATE_MOVE_CTOR_NE_ND(CLASS) J_A(ND) CLASS(CLASS && rhs) noexcept = default;
#define J_BOILERPLATE_MOVE_CTOR_CE_ND(CLASS) J_A(ND,AI) inline constexpr CLASS(CLASS && rhs) noexcept = default;
#define J_BOILERPLATE_MOVE_CTOR_DEL_ND(CLASS) J_A(ND) CLASS(CLASS && rhs) = delete;

#define J_BOILERPLATE_MOVE_ASSIGN(CLASS) CLASS & operator=(CLASS && rhs) = default;
#define J_BOILERPLATE_MOVE_ASSIGN_NE(CLASS) CLASS & operator=(CLASS && rhs) noexcept = default;
#define J_BOILERPLATE_MOVE_ASSIGN_CE(CLASS) constexpr CLASS & operator=(CLASS && rhs) noexcept = default;
#define J_BOILERPLATE_MOVE_ASSIGN_DEL(CLASS) CLASS & operator=(CLASS && rhs) = delete;

#define J_BOILERPLATE_REF_CTOR(CLASS) CLASS(CLASS & rhs) = default;
#define J_BOILERPLATE_REF_CTOR_NE(CLASS) CLASS(CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_REF_CTOR_CE(CLASS) constexpr CLASS(CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_REF_CTOR_DEL(CLASS) CLASS(CLASS & rhs) = delete;

#define J_BOILERPLATE_REF_ASSIGN(CLASS) CLASS & operator=(CLASS & rhs) = default;
#define J_BOILERPLATE_REF_ASSIGN_NE(CLASS) CLASS & operator=(CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_REF_ASSIGN_CE(CLASS) constexpr CLASS & operator=(CLASS & rhs) noexcept = default;
#define J_BOILERPLATE_REF_ASSIGN_DEL(CLASS) CLASS & operator=(CLASS & rhs) = delete;

#define J_BOILERPLATE_COPY(CLASS)    \
  J_BOILERPLATE_COPY_CTOR(CLASS)     \
  J_BOILERPLATE_COPY_ASSIGN(CLASS)

#define J_BOILERPLATE_COPY_ND(CLASS) J_BOILERPLATE_COPY_CTOR_ND(CLASS) J_BOILERPLATE_COPY_ASSIGN(CLASS)

#define J_BOILERPLATE_COPY_NE(CLASS) J_BOILERPLATE_COPY_CTOR_NE(CLASS) J_BOILERPLATE_COPY_ASSIGN_NE(CLASS)
#define J_BOILERPLATE_COPY_NE_ND(CLASS) J_BOILERPLATE_COPY_CTOR_NE_ND(CLASS) J_BOILERPLATE_COPY_ASSIGN_NE(CLASS)

#define J_BOILERPLATE_COPY_CE(CLASS) J_BOILERPLATE_COPY_CTOR_CE(CLASS) J_BOILERPLATE_COPY_ASSIGN_CE(CLASS)
#define J_BOILERPLATE_COPY_CE_ND(CLASS) J_BOILERPLATE_COPY_CTOR_CE_ND(CLASS) J_BOILERPLATE_COPY_ASSIGN_CE(CLASS)

#define J_BOILERPLATE_COPY_DEL(CLASS) \
  J_BOILERPLATE_COPY_CTOR_DEL(CLASS)

#define J_BOILERPLATE_MOVE(CLASS) J_BOILERPLATE_MOVE_CTOR(CLASS) J_BOILERPLATE_MOVE_ASSIGN(CLASS)
#define J_BOILERPLATE_MOVE_ND(CLASS) J_BOILERPLATE_MOVE_CTOR_ND(CLASS) J_BOILERPLATE_MOVE_ASSIGN(CLASS)

#define J_BOILERPLATE_MOVE_NE(CLASS) \
  J_BOILERPLATE_MOVE_CTOR_NE(CLASS)  \
  J_BOILERPLATE_MOVE_ASSIGN_NE(CLASS)

#define J_BOILERPLATE_MOVE_NE_ND(CLASS) J_BOILERPLATE_MOVE_CTOR_NE_ND(CLASS) J_BOILERPLATE_MOVE_ASSIGN_NE(CLASS)

#define J_BOILERPLATE_MOVE_CE(CLASS) \
  J_BOILERPLATE_MOVE_CTOR_CE(CLASS)  \
  J_BOILERPLATE_MOVE_ASSIGN_CE(CLASS)

#define J_BOILERPLATE_MOVE_CE_ND(CLASS) J_BOILERPLATE_MOVE_CTOR_CE_ND(CLASS) J_BOILERPLATE_MOVE_ASSIGN_CE(CLASS)

#define J_BOILERPLATE_MOVE_DEL(CLASS) J_BOILERPLATE_MOVE_CTOR_DEL(CLASS)

#define J_BOILERPLATE_REF(CLASS)    \
  J_BOILERPLATE_REF_CTOR(CLASS)     \
  J_BOILERPLATE_REF_ASSIGN(CLASS)

#define J_BOILERPLATE_REF_NE(CLASS) \
  J_BOILERPLATE_REF_CTOR_NE(CLASS)  \
  J_BOILERPLATE_REF_ASSIGN_NE(CLASS)

#define J_BOILERPLATE_REF_CE(CLASS) \
  J_BOILERPLATE_REF_CTOR_CE(CLASS)  \
  J_BOILERPLATE_REF_ASSIGN_CE(CLASS)

#define J_BOILERPLATE_REF_DEL(CLASS)    \
  J_BOILERPLATE_REF_CTOR_DEL(CLASS)     \
  J_BOILERPLATE_REF_ASSIGN_DEL(CLASS)

#define J_BOILERPLATE_ALL_DEL(CLASS)    \
  J_BOILERPLATE_CTOR_DEL(CLASS)         \
  J_BOILERPLATE_COPY_DEL(CLASS)         \
  J_BOILERPLATE_MOVE_DEL(CLASS)         \

#define J_BOILERPLATE_IMPL(TYPE, CLASS) J_BOILERPLATE_##TYPE(CLASS)
#define J_BOILERPLATE(CLASS, ...) J_FE_POSTARG(J_BOILERPLATE_IMPL, CLASS, __VA_ARGS__)

#define J_DEFAULT_COPY(CLASS) \
  CLASS(const CLASS & rhs) = default; \
  CLASS & operator=(const CLASS & rhs) = default;

#define J_DEFAULT_COPY_NE(CLASS) \
  CLASS(const CLASS & rhs) noexcept = default; \
  CLASS & operator=(const CLASS & rhs) noexcept = default;

#define J_DEFAULT_MOVE(CLASS) \
  J_ALWAYS_INLINE CLASS(CLASS && rhs) noexcept = default; \
  J_ALWAYS_INLINE CLASS & operator=(CLASS && rhs) noexcept = default;

#define J_DEFAULT_COPY_MOVE(CLASS) J_DEFAULT_COPY(CLASS) J_DEFAULT_MOVE(CLASS)
#define J_DEFAULT_COPY_NE_MOVE(CLASS) J_DEFAULT_COPY_NE(CLASS) J_DEFAULT_MOVE(CLASS)
#define J_DEFAULT_MOVE_CTOR(CLASS) CLASS() = default; J_DEFAULT_MOVE(CLASS)
#define J_DEFAULT_MOVE_CTOR_NE(CLASS) CLASS() noexcept = default; J_DEFAULT_MOVE(CLASS)
#define J_DEFAULT_COPY_MOVE_CTOR(CLASS) CLASS() = default; \
  J_DEFAULT_COPY(CLASS) J_DEFAULT_MOVE(CLASS)
#define J_DEFAULT_COPY_MOVE_CTOR_NE(CLASS) CLASS() noexcept = default; \
  J_DEFAULT_COPY(CLASS) J_DEFAULT_MOVE(CLASS)
#define J_DEFAULT_COPY_NE_MOVE_CTOR_NE(CLASS) CLASS() noexcept = default; \
  J_DEFAULT_COPY_NE(CLASS) J_DEFAULT_MOVE(CLASS)

#define J_MIN(A, B) ((A) < (B) ? (A) : (B))
#define J_MAX(A, B) ((A) < (B) ? (B) : (A))

namespace j {
  namespace detail {
    template<typename T> struct J_TYPE_HIDDEN type_identity { using type J_NO_DEBUG_TYPE = T; };
  }

  /// Resolves to T, but through a round-trip that prevents template argument deduction.
  ///
  /// E.g. the max and min functions below use this, to force the subsequent arguments to match
  /// the first argument. E.g. min(0.5f, 0) would fail template argument deduction otherwise.
  template<typename T>
  using type_identity_t J_NO_DEBUG_TYPE = typename detail::type_identity<T>::type;

  template<typename T, typename U>
  J_NO_DEBUG inline constexpr bool is_same_v = __is_same(T, U);

  template<typename T, typename... U>
  J_NO_DEBUG inline constexpr bool is_one_of_v = (__is_same(T, U) || ...);

  template<typename T>
  J_NO_DEBUG inline constexpr bool is_void_v = __is_void(T);

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T min(T a) noexcept {
    return a;
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T min(T a, typename detail::type_identity<T>::type b) noexcept {
    return a < b ? a : b;
  }

  template<>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN float min<float>(float a, float b) noexcept {
    return __builtin_fminf(a, b);
  }

  template<>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN double min<double>(double a, double b) noexcept {
    return __builtin_fmin(a, b);
  }

  template<typename T, typename... Ts>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T min(T a, typename detail::type_identity<T>::type b, Ts... vs) noexcept {
    if constexpr (is_same_v<T, float>) {
      return min(__builtin_fminf(a, b), vs...);
    } else if constexpr (is_same_v<T, double>) {
      return min(__builtin_fmin(a, b), vs...);
    } else {
      return min(a < b ? a : b, vs...);
    }
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG constexpr T max(T a) noexcept {
    return a;
  }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG constexpr T max(T a, typename detail::type_identity<T>::type b) noexcept {
    return a < b ? b : a;
  }

  template<>
  J_INLINE_GETTER_NO_DEBUG float max<float>(float a, float b) noexcept {
    return __builtin_fmaxf(a, b);
  }

  template<>
  J_INLINE_GETTER_NO_DEBUG double max<double>(double a, double b) noexcept {
    return __builtin_fmax(a, b);
  }

  template<typename T, typename... Ts>
  J_INLINE_GETTER_NO_DEBUG constexpr T max(T a, typename detail::type_identity<T>::type b, Ts... vs) noexcept {
    if constexpr (is_same_v<T, float>) {
      return max(__builtin_fmaxf(a, b), vs...);
    } else if constexpr (is_same_v<T, double>) {
      return max(__builtin_fmax(a, b), vs...);
    } else {
      return max(a < b ? b : a, vs...);
    }
  }

  /// Clamps the middle argument between surrounding ones.
  ///
  /// For correct behavior, min must be less than max.
  template<typename T>
  J_A(AI,ND,HIDDEN,NODISC) inline T clamp(typename detail::type_identity<T>::type min, T value, typename detail::type_identity<T>::type max) noexcept {
    return min > value ? min : (max < value ? max : value);
  }

  template<>
  J_A(AI,ND,HIDDEN,NODISC,FLATTEN) inline float clamp<float>(float min, float value, float max) noexcept {
    return __builtin_fmaxf(min, __builtin_fminf(max, value));
  }

  template<>
  J_A(AI,ND,HIDDEN,NODISC,FLATTEN) inline double clamp<double>(double min, double value, double max) noexcept {
    return __builtin_fmax(min, __builtin_fmin(max, value));
  }

  J_A(AI,ND,HIDDEN,NODISC,FLATTEN) inline float abs(float a) noexcept {
    return __builtin_fabsf(a);
  }

  J_A(AI,ND,HIDDEN,NODISC,FLATTEN) inline double abs(double a) noexcept {
    return __builtin_fabs(a);
  }

  J_A(AI,ND,HIDDEN,NODISC,FLATTEN) inline i32_t abs(i32_t a) noexcept {
    return __builtin_abs(a);
  }

  J_A(AI,ND,HIDDEN,NODISC,FLATTEN) inline i64_t abs(i64_t a) noexcept {
    return __builtin_labs(a);
  }

  template<typename T>
  J_NO_DEBUG constexpr inline u8_t bitsizeof = sizeof(T) * 8U;

  template<typename T, T... Is>
  struct J_TYPE_HIDDEN integer_sequence { };

  template<sz_t N, typename T = sz_t>
  using make_index_sequence J_NO_DEBUG_TYPE = __make_integer_seq<::j::integer_sequence, T, N>;

  /// Function that returns its argument, but only eats values in non-evaluated contexts.
  ///
  /// Can be used to make a poor man's decay - although that assumes that the type is
  //// copy-constructible.
  template <typename T> J_A(AI,ND,HIDDEN) inline static T identity(T) noexcept;

  /// Function for consuming arguments by value for non-evaluated contexts.
  ///
  /// This is especially useful when determining whether a perfect-forwarded argument can be
  /// nothrow move/copy constructed:
  ///
  /// `template<typename T> T foo(T && v) noexcept(noexcept(consume(declval<T>())))`
  ///
  /// \note The noexcept behavior has a slight caveat - if destructing T throws, the function
  ///       will not be noexcept (as caller destructs pass-by-value arguments). Since destructors
  ///       cannot safely throw either way, this likely does not matter.
  template <typename... Ts> J_A(ND,HIDDEN) static inline  void consume(Ts...) noexcept;

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T && move(T & v) noexcept { return static_cast<T &&>(v); }

  template<typename T>
  J_INLINE_GETTER_NO_DEBUG_FLATTEN constexpr T && forward(T && v) noexcept { return static_cast<T &&>(v); }

  namespace detail {
    template<bool>
    struct J_TYPE_HIDDEN conditional {
      template<typename T, typename> using type J_NO_DEBUG_TYPE = T;
    };
    template<> struct J_TYPE_HIDDEN conditional<false> {
      template<typename, typename F> using type J_NO_DEBUG_TYPE = F;
    };

    template<bool, typename T>
    struct J_TYPE_HIDDEN const_if final {
      using type J_NO_DEBUG_TYPE = T;
    };
    template<typename T>
    struct J_TYPE_HIDDEN const_if<true, T> final {
      using type J_NO_DEBUG_TYPE = const T;
    };
    template<typename T, u32_t>
    using repeat_arg_helper_t J_NO_DEBUG_TYPE = T;

    template<typename T, template<typename...> typename U>
    struct J_TYPE_HIDDEN repeat_helper final {
      template<typename, u32_t... Is>
      using type J_NO_DEBUG_TYPE = U<repeat_arg_helper_t<T, Is>...>;
    };
  }

  template<template<typename...> typename Temp, typename ArgT, u32_t N>
  using repeat_t J_NO_DEBUG_TYPE = __make_integer_seq<detail::repeat_helper<ArgT, Temp>::template type, ArgT, N>;

  template<bool Value, typename True, typename False>
  using conditional_t J_NO_DEBUG_TYPE = typename detail::conditional<Value>::template type<True, False>;

  template<bool Value, typename T>
  using const_if_t J_NO_DEBUG_TYPE = typename detail::const_if<Value, T>::type;

  template<typename A, typename B>
  J_A(AI,ND,NODISC,HIDDEN) inline iptr_t byte_offset(const A * J_NOT_NULL a, const B * J_NOT_NULL b) noexcept {
    return (const char *)b - (const char *)a;
  }

  template<typename Result = void, typename A>
  J_A(AI,ND,NODISC,HIDDEN,RNN) auto * add_bytes(A * J_NOT_NULL ptr, i32_t offset) noexcept {
    return reinterpret_cast<conditional_t<__is_void(Result), A*, Result>>(
      const_cast<char*>(reinterpret_cast<const char *>(ptr) + offset));
  }

  template<typename Result = void, typename A>
  J_A(AI,ND,NODISC,HIDDEN,RNN) auto * subtract_bytes(A * J_NOT_NULL ptr, i32_t offset) noexcept {
    return reinterpret_cast<conditional_t<__is_void(Result), A*, Result>>(
      const_cast<char*>(reinterpret_cast<const char *>(ptr) - offset));
  }

  template<typename To, typename From>
  J_A(AI,ND,NODISC,HIDDEN) constexpr To bit_cast(From from) noexcept {
    return __builtin_bit_cast(To, from);
  }

  template<typename T> struct type_tag final { using type J_NO_DEBUG_TYPE = T; };
  template<typename T> J_NO_DEBUG constexpr inline type_tag<T> type;

  template<auto T> struct value_tag final { };
  template<auto T> J_NO_DEBUG constexpr inline value_tag<T> value_tag_v;

  template<typename T, typename> using first_t = T;
}
