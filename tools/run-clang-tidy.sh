#!/bin/sh

env

cd "${MESON_SOURCE_ROOT}"

#jq 'map(select(.file |(startswith("../src") or startswith("../test") or (startswith(".")|not))))' compile_commands.json > compile_commands_filtered.json


checks=(
 -clang-analyzer-alpha.fuchsia.*
 -clang-analyzer-alpha.security.cert.*
 -clang-analyzer-alpha.unix.PthreadLock
 -clang-analyzer-alpha.unix.Chroot
 -clang-analyzer-alpha.unix.BlockInCriticalSection

 clang-analyzer-alpha.unix.cstring.OutOfBounds
 clang-analyzer-alpha.unix.StdCLibraryFunctionArgs
 -clang-analyzer-alpha.deadcode.UnreachableCode
 clang-analyzer-alpha.cplusplus.EnumCastOutOfRange
 clang-analyzer-alpha.core.TestAfterDivZero
 clang-analyzer-alpha.core.DynamicTypeChecker

  clang-analyzer-alpha.core.FixedAddr
  clang-analyzer-alpha.core.IdenticalExpr

  clang-analyzer-alpha.core.CastSize
  clang-analyzer-alpha.core.CastToStruct
  clang-analyzer-alpha.core.Conversion
      -clang-analyzer-alpha.cplusplus.STLAlgorithmModeling
    -clang-analyzer-alpha.cplusplus.SmartPtr


  -clang-analyzer-alpha.cplusplus.InvalidatedIterator
  -clang-analyzer-alpha.cplusplus.IteratorModeling
  -clang-analyzer-alpha.cplusplus.IteratorRange
  -clang-analyzer-alpha.cplusplus.MismatchedIterator
  -clang-analyzer-alpha.cplusplus.ContainerModeling


  'readability-static-definition-in-anonymous-namespace'
  'readability-static-accessed-through-instance'
  'readability-simplify-boolean-expr'
  'readability-redundant-preprocessor'
  'readability-redundant-member-init'
  'readability-redundant-function-ptr-dereference'
  'readability-redundant-declaration'
  'readability-redundant-control-flow'
  'readability-redundant-access-specifiers'
  'readability-non-const-parameter'
  'readability-misplaced-array-index'
  'readability-misleading-indentation'
  #'readability-isolate-declaration'
  'readability-inconsistent-declaration-parameter-name'
  #'readability-function-size'
  #'readability-function-cognitive-complexity'
  'readability-delete-null-pointer'
  'readability-avoid-const-params-in-decls'

  performance-implicit-conversion-in-loop
  performance-move-const-arg
  performance-move-constructor-init
  performance-no-automatic-move
  #performance-no-int-to-ptr
  #performance-noexcept-move-constructor
  performance-trivially-destructible
  performance-type-promotion-in-math-fn
  performance-unnecessary-copy-initialization
  performance-unnecessary-value-param

  clang-analyzer-optin.performance.Padding
  #'clang-analyzer-optin.cplusplus.UninitializedObject'
  clang-analyzer-optin.cplusplus.VirtualCall

  # modernize-use-trailing-return-type
  modernize-use-nullptr
  modernize-use-nodiscard
  modernize-use-noexcept
  modernize-use-bool-literals
  modernize-pass-by-value
  modernize-concat-nested-namespaces
  modernize-deprecated-headers

  misc-unused-alias-decls
  misc-unused-parameters
  misc-unused-using-decls
  misc-redundant-expression
  misc-misplaced-const
  misc-definitions-in-headers

  # llvm-include-order
  llvm-header-guard
  llvm-twine-local

  hicpp-invalid-access-moved
  hicpp-multiway-paths-covered
  # hicpp-signed-bitwise


  google-readability-todo
  google-global-names-in-headers

  #cppcoreguidelines-narrowing-conversions

  -clang-analyzer-security.insecureAPI.vfork
  -clang-analyzer-security.insecureAPI.bcmp
  -clang-analyzer-security.insecureAPI.bcopy
  -clang-analyzer-security.insecureAPI.bzero
  -clang-analyzer-security.insecureAPI.decodeValueOfObjCType
  -clang-analyzer-security.insecureAPI.getpw
  -clang-analyzer-security.insecureAPI.gets
  -clang-analyzer-security.insecureAPI.mkstemp
  -clang-analyzer-security.insecureAPI.mktemp
  -clang-analyzer-security.insecureAPI.rand
  -clang-analyzer-security.insecureAPI.strcpy

  -clang-analyzer-unix.Vfork
  clang-analyzer-cplusplus.InnerPointer
  clang-analyzer-cplusplus.Move
  clang-analyzer-cplusplus.NewDelete
  clang-analyzer-cplusplus.NewDeleteLeaks
  clang-analyzer-cplusplus.VirtualCallModeling
  clang-analyzer-cplusplus.PureVirtualCall

  clang-analyzer-deadcode.DeadStores

  clang-analyzer-core.uninitialized.*
  clang-analyzer-core.DynamicTypePropagation
  clang-analyzer-core.NonNullParamChecker
  clang-analyzer-core.NonnilStringConstants
  clang-analyzer-core.NullDereference
  clang-analyzer-core.StackAddrEscapeBase
  clang-analyzer-core.StackAddressEscape
  clang-analyzer-core.UndefinedBinaryOperatorResult


  bugprone-too-small-loop-variable
  bugprone-undefined-memory-manipulation
  bugprone-virtual-near-miss
  bugprone-use-after-move


  bugprone-suspicious-enum-usage
  bugprone-spuriously-wake-up-functions
  bugprone-sizeof-expression
  bugprone-redundant-branch-condition
  #bugprone-reserved-identifier
  #bugprone-narrowing-conversions
  bugprone-no-escape
  bugprone-signed-char-misuse
  bugprone-misplaced-operator-in-strlen-in-alloc
  bugprone-misplaced-pointer-arithmetic-in-alloc
  bugprone-misplaced-widening-cast
  bugprone-move-forwarding-reference
      bugprone-incorrect-roundings
    bugprone-infinite-loop
    bugprone-integer-division
    bugprone-lambda-function-name
        bugprone-fold-init-type
    bugprone-forward-declaration-namespace
    bugprone-forwarding-reference-overload

        bugprone-branch-clone
    bugprone-copy-constructor-init
    bugprone-dangling-handle
    bugprone-dynamic-static-initializers




    bugprone-suspicious-include
    bugprone-suspicious-memset-usage
    bugprone-suspicious-missing-comma
    bugprone-suspicious-semicolon



  # Disables
  '-clang-analyzer-osx.*'
  '-clang-analyzer-fuchsia.*'
  '-clang-analyzer-webkit.*'
  '-clang-analyzer-apiModeling.google.GTest'
  -clang-analyzer-alpha.clone.CloneChecker
  -clang-analyzer-alpha.core.PointerArithm
)


checks=$(IFS=,;echo "${checks[*]}")
echo $checks

#clang-tidy \
#  --dump-config \
#  --quiet \
#  --allow-enabling-analyzer-alpha-checkers \
#  --checks="$checks" \
#  -p="${MESON_BUILD_ROOT}" \
#  "${MESON_SOURCE_ROOT}\/src\/.*\.cpp$" \
#  "${MESON_SOURCE_ROOT}\/test\/.*\.cpp$"

  # "${MESON_SOURCE_ROOT}\/test\/.*\.cpp$" 2>&1 \
/usr/share/clang/run-clang-tidy.py \
  -quiet \
  -allow-enabling-alpha-checkers \
  -checks="$checks" \
  -p="${MESON_BUILD_ROOT}" \
  "${MESON_SOURCE_ROOT}\/src\/.*\.cpp$" \
  | grep -v '^clang-tidy' \
  | grep -v '^[0-9][0-9]* warnings  *generated'
