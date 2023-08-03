#pragma once

// Since Apple put a big willie on the implementation of features from C++20, so I can't use part of
// the features from C++20. What could be better than that?
#if SALT_TARGET_MACOSX && SALT_CLANG_FULL_VERSION <= 130106
#    define SALT_HAS_NO_RANGES (1)
#endif

#define SALT_HAS_ATTRIBUTE_ASSUME            (0)
#define SALT_HAS_ATTRIBUTE_ASSUME_ALIGNED    (0)
#define SALT_HAS_ATTRIBUTE_DLLIMPORT         (0)
#define SALT_HAS_ATTRIBUTE_STDCALL           (0)
#define SALT_HAS_ATTRIBUTE_NO_UNIQUE_ADDRESS (0)

#if __has_cpp_attribute(clang::assume)
#    undef SALT_HAS_ATTRIBUTE_ASSUME
#    define SALT_HAS_ATTRIBUTE_ASSUME (1)
#endif

#if __has_cpp_attribute(gnu::assume_aligned)
#    undef SALT_HAS_ATTRIBUTE_ASSUME_ALIGNED
#    define SALT_HAS_ATTRIBUTE_ASSUME_ALIGNED (1)
#endif

#if __has_cpp_attribute(gnu::dllimport) && !defined(__WINE__)
#    undef SALT_HAS_ATTRIBUTE_DLLIMPORT
#    define SALT_HAS_ATTRIBUTE_DLLIMPORT (1)
#endif

#if __has_cpp_attribute(gnu::stdcall) && !defined(__WINE__)
#    undef SALT_HAS_ATTRIBUTE_STDCALL
#    define SALT_HAS_ATTRIBUTE_STDCALL (1)
#endif

#if __has_cpp_attribute(no_unique_address)
#    undef SALT_HAS_ATTRIBUTE_NO_UNIQUE_ADDRESS
#    define SALT_HAS_ATTRIBUTE_NO_UNIQUE_ADDRESS (1)
#endif

#define SALT_HAS_ATTRIBUTE(X) SALT_JOIN(SALT_HAS_ATTRIBUTE_, X)
