/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_UTILS_H
#define KAI_UTILS_H

#include <assert.h>
#include <type_traits>

#include "types.h"

#define KAI_ASSERT(expr) assert(expr)
#define KAI_IGNORED_VARIABLE(v) (void)v

#ifdef KAI_PLATFORM_WIN32
#define KAI_FORCEINLINE __forceinline
#else
#define KAI_FORCEINLINE inline
#endif

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

    template<typename T>
    KAI_FORCEINLINE T min(T a, T b) {
        return (a <= b) ? a : b;
    }

    template<typename T>
    KAI_FORCEINLINE T max(T a, T b) {
        return (a >= b) ? a : b;
    }

    template<typename T>
    KAI_FORCEINLINE T abs(T val) {
        return (val >= 0) ? val : -val;
    }
}

#endif /* KAI_UTILS_H */
