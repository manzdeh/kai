/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_UTILS_H
#define KAI_UTILS_H

#include <assert.h>
#include <type_traits>

#include "types.h"

#ifdef KAI_PLATFORM_WIN32

#define KAI_API __declspec(dllexport)
#define KAI_FORCEINLINE __forceinline

// NOTE: This *MUST* always be the last member in a struct and there can only be *ONE* of these per struct
#define KAI_FLEXIBLE_ARRAY(name) name[]

#define KAI_PUSH_DISABLE_COMPILER_WARNINGS(...) \
    __pragma(warning(push)) \
    __pragma(warning(disable: __VA_ARGS__))
#define KAI_POP_COMPILER_WARNINGS __pragma(warning(pop))

#else

#define KAI_FORCEINLINE inline
#define KAI_PUSH_DISABLE_COMPILER_WARNINGS(...)
#define KAI_POP_COMPILER_WARNINGS
#error Implement KAI_FLEXIBLE_ARRAY for this compiler

#endif

#define KAI_ASSERT(expr) assert(expr)

#define KAI_IGNORED_VARIABLE(v) (void)v

#define KAI_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#define KAI_TOKEN_TO_STRING_HELPER(tok) #tok
#define KAI_TOKEN_TO_STRING(tok) KAI_TOKEN_TO_STRING_HELPER(tok)

namespace kai {
    constexpr Uint64 kibibytes(Uint64 value) {
        return value * 1024ull;
    }

    constexpr Uint64 mebibytes(Uint64 value) {
        return kibibytes(value) * 1024ull;
    }

    constexpr Uint64 gibibytes(Uint64 value) {
        return mebibytes(value) * 1024ull;
    }

    template<typename T>
    KAI_FORCEINLINE bool is_pow2(T value) {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                      "Value can only be an unsigned integer");
        return (value > 0) && (((value - 1) & value) == 0);
    }

    // NOTE: 'alignment' must be a power of two
    template<typename T>
    KAI_FORCEINLINE void align_to_pow2(T &value, T alignment) {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                      "Value can only be an unsigned integer");
        KAI_ASSERT(is_pow2(alignment));
        alignment--;
        value = (value + alignment) & ~alignment;
    }
}

#endif /* KAI_UTILS_H */
