#pragma once

#if defined(SALT_CLANG_FULL_VER) && SALT_CLANG_FULL_VER < 140000
#    define SALT_HAS_NO_CONCEPTS (1)
#elif defined(SALT_MSVC_FULL_VER) && SALT_MSVC_FULL_VER < 192930134
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