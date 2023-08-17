#pragma once

// Since Apple put a big willie on the implementation of features from C++20, so I can't use part of
// the features from C++20. What could be better than that?
#if SALT_TARGET_MACOSX && SALT_CLANG_FULL_VERSION <= 130106
#    define SALT_HAS_NO_RANGES (1)
#endif

#define SALT_HAS_ATTRIBUTE_ASSUME            (0)
#define SALT_HAS_ATTRIBUTE_ASSUME_ALIGNED    (0)
#define SALT_HAS_ATTRIBUTE_ALWAYS_INLINE     (0)
#define SALT_HAS_ATTRIBUTE_DLLIMPORT         (0)
#define SALT_HAS_ATTRIBUTE_STDCALL           (0)

#if __has_cpp_attribute(clang::assume)
#    undef SALT_HAS_ATTRIBUTE_ASSUME
#    define SALT_HAS_ATTRIBUTE_ASSUME (1)
#endif

#if __has_cpp_attribute(gnu::assume_aligned)
#    undef SALT_HAS_ATTRIBUTE_ASSUME_ALIGNED
#    define SALT_HAS_ATTRIBUTE_ASSUME_ALIGNED (1)
#endif

#if __has_cpp_attribute(clang::always_inline)
#    undef SALT_HAS_ATTRIBUTE_ALWAYS_INLINE
#    define SALT_HAS_ATTRIBUTE_ALWAYS_INLINE (1)
#endif

#if __has_cpp_attribute(gnu::dllimport) && !defined(__WINE__)
#    undef SALT_HAS_ATTRIBUTE_DLLIMPORT
#    define SALT_HAS_ATTRIBUTE_DLLIMPORT (1)
#endif

#if __has_cpp_attribute(gnu::stdcall) && !defined(__WINE__)
#    undef SALT_HAS_ATTRIBUTE_STDCALL
#    define SALT_HAS_ATTRIBUTE_STDCALL (1)
#endif

#define SALT_HAS_ATTRIBUTE(X) SALT_JOIN(SALT_HAS_ATTRIBUTE_, X)

#if __has_cpp_attribute(msvc::no_unique_address)
// MSVC implements [[no_unique_address]] as a silent no-op currently. If/when MSVC breaks its C++
// ABI, it will be changed to work as intended. However, MSVC implements [[msvc::no_unique_address]]
// which does what [[no_unique_address]] is supposed to do, in general.

// Clang-cl does not yet (14.0) implement either [[no_unique_address]] or
// [[msvc::no_unique_address]] though. If/when it does implement [[msvc::no_unique_address]], this
// should be preferred though.
#    define SALT_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif __has_cpp_attribute(no_unique_address)
#    define SALT_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
// Note that this can be replaced by #error as soon as clang-cl implements msvc::no_unique_address,
// since there should be no C++20 compiler that doesn't support one of the two attributes at that
// point. I generally don't want to use this macro outside of C++20-only code, because using it
// conditionally in one language version only would make the ABI inconsistent.
#    define SALT_NO_UNIQUE_ADDRESS /* nothing */
#endif