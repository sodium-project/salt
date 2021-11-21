#pragma once

// For compatibility with existing code that compiles with MSVC, clang defines the _MSC_VER and _MSC_FULL_VER macros.
#if defined(SALT_CLANG) && defined(_MSC_VER)
#    define SALT_MSVC_FULL_VER (_MSC_FULL_VER)
#elif defined(SALT_CLANG) && defined(__clang__)
#    define SALT_CLANG_FULL_VER (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#else
#    error "Compiler not supported."
#endif