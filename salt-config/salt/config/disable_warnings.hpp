#pragma once

// clang-format off
#if defined(SALT_CLANG)
#    define _SALT_PRAGMA(x)                              _Pragma(SALT_TO_STRING(x))
#    define SALT_DISABLE_WARNING_PUSH                    _SALT_PRAGMA(clang diagnostic push)
#    define SALT_DISABLE_WARNING_POP                     _SALT_PRAGMA(clang diagnostic pop)
#    define SALT_DISABLE_WARNING(warning)                _SALT_PRAGMA(clang diagnostic ignored #warning)
#    define SALT_DISABLE_WARNING_BLOCK(warning, ...)                                                        \
         SALT_DISABLE_WARNING_PUSH                                                                          \
         SALT_DISABLE_WARNING(warning)                                                                      \
         __VA_ARGS__                                                                                        \
         SALT_DISABLE_WARNING_POP
// Warnings you want to deactivate:
#    define SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS SALT_DISABLE_WARNING(-Wdeprecated-declarations)
#    define SALT_DISABLE_WARNING_MICROSOFT_TEMPLATE      SALT_DISABLE_WARNING(-Wmicrosoft-template)
#else
#    define SALT_DISABLE_WARNING_PUSH                    /* nothing */
#    define SALT_DISABLE_WARNING_POP                     /* nothing */
#    define SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS /* nothing */
#endif
// clang-format on