#pragma once

// For compatibility with existing code that compiles with MSVC, clang defines the _MSC_VER and _MSC_FULL_VER macros.
#if defined(SALT_CLANG) && defined(_MSC_VER)
#    define SALT_MSVC_FULL_VER (_MSC_FULL_VER)
#elif defined(SALT_CLANG) && defined(__clang__)
#    define SALT_CLANG_FULL_VER (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#else
#    error "Compiler not supported."
#endif

// clang-format off
#if defined(SALT_CLANG)
#    define _SALT_PRAGMA(X)               _Pragma(#X)
#    define SALT_DISABLE_WARNING_PUSH     _SALT_PRAGMA(GCC diagnostic push)
#    define SALT_DISABLE_WARNING_POP      _SALT_PRAGMA(GCC diagnostic pop)
#    define SALT_DISABLE_WARNING(warning) _SALT_PRAGMA(GCC diagnostic ignored #warning)
// warnings you want to deactivate...
#    define SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS SALT_DISABLE_WARNING(-Wdeprecated-declarations)
#else
#    define SALT_DISABLE_WARNING_PUSH
#    define SALT_DISABLE_WARNING_POP
#    define SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS
#endif
// clang-format on

#if defined(SALT_CLANG_FULL_VER) && SALT_CLANG_FULL_VER < 140000
#    define SALT_HAS_NO_CONCEPTS (1)
#elif defined(SALT_MSVC_FULL_VER) && SALT_MSVC_FULL_VER < 192930136
#    define SALT_HAS_NO_CONCEPTS (1)
#endif

#if defined(SALT_CLANG_FULL_VER) && SALT_CLANG_FULL_VER < 140000
#    define SALT_HAS_NO_CONSTEVAL (1)
#elif defined(SALT_MSVC_FULL_VER) && SALT_MSVC_FULL_VER < 193030704
#    define SALT_HAS_NO_CONSTEVAL (1)
#endif

#if defined(SALT_CLANG_FULL_VER) && SALT_CLANG_FULL_VER < 140000
#    define SALT_HAS_NO_SOURCE_LOCATION (1)
#elif defined(SALT_MSVC_FULL_VER) && SALT_MSVC_FULL_VER < 193030704
#    define SALT_HAS_NO_SOURCE_LOCATION (1)
#endif

#if defined(SALT_HAS_NO_CONSTEVAL)
#    define SALT_CONSTEVAL constexpr
#else
#    define SALT_CONSTEVAL consteval
#endif