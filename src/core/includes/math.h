/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_MATH_H
#define KAI_MATH_H

#include "utils.h"
#include "types.h"

namespace kai {
    template<typename T>
    KAI_FORCEINLINE T min(T a, T b) { return (a <= b) ? a : b; }
    template<typename T>
    KAI_FORCEINLINE T max(T a, T b) { return (a >= b) ? a : b; }
    template<typename T>
    KAI_FORCEINLINE T abs(T val) { return (val >= 0) ? val : -val; }
    template<typename T>
    KAI_FORCEINLINE void clamp(T &value, T v0, T v1) {
        float t = min(v0, v1);
        v1 = max(v0, v1);
        v0 = t;
        value = min(max(v0, value), v1);
    }

    struct Vec2 {
        Vec2(Float32 x = 0.0f, Float32 y = 0.0f) : x(x), y(y) {}

        Float32 x;
        Float32 y;
    };
}

#endif /* KAI_MATH_H */
