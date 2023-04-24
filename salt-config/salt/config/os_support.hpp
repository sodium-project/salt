#pragma once

#define SALT_TARGET_APPLE   (0)
#define SALT_TARGET_MACOSX  (0)
#define SALT_TARGET_WINDOWS (0)
#define SALT_TARGET_LINUX   (0)

// clang-format off
#if defined(_WIN64)
#    undef  SALT_TARGET_WINDOWS
#    define SALT_TARGET_WINDOWS (1)
#elif defined(__APPLE__)
#    undef  SALT_TARGET_APPLE
#    define SALT_TARGET_APPLE (1)
#    include <TargetConditionals.h>
#    if defined(TARGET_OS_MAC)
#        undef  SALT_TARGET_MACOSX
#        define SALT_TARGET_MACOSX (1)
#    endif
#elif defined(__linux__)
#    undef  SALT_TARGET_LINUX
#    define SALT_TARGET_LINUX (1)
#endif
// clang-format on

#define SALT_TARGET(X) SALT_JOIN(SALT_TARGET_, X)