#pragma once

#if defined(SALT_CLANG_FULL_VERSION) && SALT_CLANG_FULL_VERSION < 140000
#    define SALT_HAS_NO_CONCEPTS (1)
#elif defined(SALT_MSC_FULL_VERSION) && SALT_MSC_FULL_VERSION < 192930134
#    define SALT_HAS_NO_CONCEPTS (1)
#endif

#if defined(SALT_CLANG_FULL_VERSION) && (SALT_TARGET(APPLE) || SALT_CLANG_FULL_VERSION < 140000)
#    define SALT_HAS_NO_CONSTEVAL (1)
#elif defined(SALT_MSC_FULL_VERSION) && SALT_MSC_FULL_VERSION < 193030704
#    define SALT_HAS_NO_CONSTEVAL (1)
#endif

#if defined(SALT_CLANG_FULL_VERSION) && (SALT_TARGET(APPLE) || SALT_CLANG_FULL_VERSION < 140000)
#    define SALT_HAS_NO_SOURCE_LOCATION (1)
#elif defined(SALT_MSC_FULL_VERSION) && SALT_MSC_FULL_VERSION < 193030704
#    define SALT_HAS_NO_SOURCE_LOCATION (1)
#endif

#if defined(SALT_HAS_NO_CONSTEVAL)
#    define SALT_CONSTEVAL constexpr
#else
#    define SALT_CONSTEVAL consteval
#endif

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

// Since Apple put a big willie on the implementation of features from C++20, so I can't use part of
// the features from C++20. What could be better than that?
#if SALT_TARGET_MACOSX && SALT_CLANG_FULL_VERSION <= 130106
#    define SALT_HAS_NO_RANGES (1)
#endif