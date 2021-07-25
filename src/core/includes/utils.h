/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_UTILS_H
#define KAI_UTILS_H

#include <assert.h>

#include "types.h"

#define KAI_ASSERT(expr) assert(expr)
#define KAI_IGNORED_VARIABLE(v) (void)v

#ifdef _WIN32
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

    KAI_FORCEINLINE bool is_pow2(Uint64 value) {
        return (value > 0) && (((value - 1) & value) == 0);
    }

    // NOTE: 'alignment' must be a power of two
    KAI_FORCEINLINE void align_to_pow2(Uint64 &value, Uint64 alignment) {
        KAI_ASSERT(is_pow2(alignment));
        alignment--;
        value = (value + alignment) & ~alignment;
    }
}

#endif /* KAI_UTILS_H */
