#pragma once

// clang-format off
#if defined(SALT_CLANG)
#    define _SALT_PRAGMA(x)               _Pragma(SALT_TO_STRING(x))
#    define SALT_DISABLE_WARNING_PUSH     _SALT_PRAGMA(GCC diagnostic push)
#    define SALT_DISABLE_WARNING_POP      _SALT_PRAGMA(GCC diagnostic pop)
#    define SALT_DISABLE_WARNING(warning) _SALT_PRAGMA(GCC diagnostic ignored #warning)
// warnings you want to deactivate...
#    define SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS SALT_DISABLE_WARNING(-Wdeprecated-declarations)
#    define SALT_DISABLE_WARNING_MICROSOFT_TEMPLATE      SALT_DISABLE_WARNING(-Wmicrosoft-template)
#else
#    define SALT_DISABLE_WARNING_PUSH
#    define SALT_DISABLE_WARNING_POP
#    define SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS
#endif
// clang-format on