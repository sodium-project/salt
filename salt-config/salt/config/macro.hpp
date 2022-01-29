#pragma once

// clang-format off
#define _SALT_JOIN(a, b) a##b
#define SALT_JOIN(a, b)  _SALT_JOIN(a, b)

#define SALT_UNIQUE_NAME(base) SALT_JOIN(base, __COUNTER__)

#define _SALT_TO_STRING(x) #x
#define SALT_TO_STRING(x)  _SALT_TO_STRING(x)
// clang-format on