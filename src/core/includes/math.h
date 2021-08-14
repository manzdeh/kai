/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_MATH_H
#define KAI_MATH_H

#include "types.h"

#include <float.h>
#include <math.h>
#include <string.h>

namespace kai {
    template<typename T>
    KAI_FORCEINLINE T min(T a, T b) { return (a <= b) ? a : b; }
    template<typename T>
    KAI_FORCEINLINE T max(T a, T b) { return (a >= b) ? a : b; }
    template<typename T>
    KAI_FORCEINLINE T abs(T val) { return (val >= 0) ? val : -val; }
    template<typename T>
    KAI_FORCEINLINE void clamp(T &value, T v0, T v1) {
        Float32 t = min(v0, v1);
        v1 = max(v0, v1);
        v0 = t;
        value = min(max(v0, value), v1);
    }
    template<typename T>
    KAI_FORCEINLINE void swap(T &lhs, T &rhs) {
        T temp = lhs;
        lhs = rhs;
        rhs = temp;
    }

    bool nearly_equal(Float32 a, Float32 b) {
        return abs(a - b) <= FLT_EPSILON;
    }

    Float32 square_root(Float32 value) {
        return sqrtf(value);
    }

    template<typename T>
    Float32 magnitude(const T &vec) {
        return square_root((vec * vec).sum());
    }

    template<typename T>
    void normalize(T &vec) {
        Float32 mag = magnitude(vec);
        if(mag != 0.0f) {
            vec /= mag;
        }
    }

    template<typename T>
    Float32 dot(const T &a, const T &b) {
        return (a * b).sum();
    }

    struct Vec2 {
        Vec2() = default;
        Vec2(Float32 x, Float32 y = 0.0f) : x(x), y(y) {}

        bool operator==(const Vec2 &rhs) const {
            return nearly_equal(x, rhs.x) && nearly_equal(y, rhs.y);
        }

        bool operator!=(const Vec2 &rhs) const {
            return !(*this == rhs);
        }

        void operator/=(Float32 scalar) {
            x /= scalar;
            y /= scalar;
        }

        Float32 sum(void) const {
            return x + y;
        }

        Float32 magnitude(void) const { return kai::magnitude(*this); }
        void normalize(void) { kai::normalize(*this); }
        Float32 dot(const Vec2 &other) const { return kai::dot(*this, other); }

        Float32 x = 0.0f;
        Float32 y = 0.0f;
    };

    Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs) {
        return { lhs.x + rhs.x, lhs.y + rhs.y };
    }

    Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs) {
        return { lhs.x - rhs.x, lhs.y - rhs.y };
    }

    Vec2 operator*(const Vec2 &lhs, const Vec2 &rhs) {
        return { lhs.x * rhs.x, lhs.y * rhs.y };
    }

    Vec2 operator/(const Vec2 &lhs, const Vec2 &rhs) {
        return { lhs.x / rhs.x, lhs.y / rhs.y };
    }

    struct Vec4 {
        Vec4() = default;
        Vec4(Float32 x, Float32 y = 0.0f, Float32 z = 0.0f, Float32 w = 0.0f) : x(x), y(y), z(z), w(w) {}

        bool operator==(const Vec4 &rhs) const {
            return nearly_equal(x, rhs.x) && nearly_equal(y, rhs.y) &&
                nearly_equal(z, rhs.z) && nearly_equal(w, rhs.w);
        }

        bool operator!=(const Vec4 &rhs) const {
            return !(*this == rhs);
        }

        void operator/=(Float32 scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
        }

        Float32 sum(void) const {
            return x + y + z + w;
        }

        Float32 magnitude(void) const { return kai::magnitude(*this); }
        void normalize(void) { kai::normalize(*this); }
        Float32 dot(const Vec4 &other) const { return kai::dot(*this, other); }

        // For the cross product the vectors are treated as 3D vectors instead,
        // because the cross product doesn't really exist for 4D vectors
        Vec4 cross(const Vec4 &rhs);

        Float32 x = 0.0f;
        Float32 y = 0.0f;
        Float32 z = 0.0f;
        Float32 w = 0.0f;
    };

    Vec4 operator+(const Vec4 &lhs, const Vec4 &rhs) {
        return {
            lhs.x + rhs.x, lhs.y + rhs.y,
            lhs.z + rhs.z, lhs.w + rhs.w
        };
    }

    Vec4 operator-(const Vec4 &lhs, const Vec4 &rhs) {
        return {
            lhs.x - rhs.x, lhs.y - rhs.y,
            lhs.z - rhs.z, lhs.w - rhs.w
        };
    }

    Vec4 operator*(const Vec4 &lhs, const Vec4 &rhs) {
        return {
            lhs.x * rhs.x, lhs.y * rhs.y,
            lhs.z * rhs.z, lhs.w * rhs.w
        };
    }

    Vec4 operator/(const Vec4 &lhs, const Vec4 &rhs) {
        return {
            lhs.x / rhs.x, lhs.y / rhs.y,
            lhs.z / rhs.z, lhs.w / rhs.w
        };
    }

    Vec4 cross(const Vec4 &a, const Vec4 &b) {
        return Vec4(a.y * b.z - a.z * b.y,
                    a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x,
                    0.0f);
    }

    Vec4 Vec4::cross(const Vec4 &rhs) {
        return kai::cross(*this, rhs);
    }

    union Mat4x4 {
        Mat4x4() {
            memset(m, 0, sizeof(m));
        }

        // Matrices need to be provided in row-major order. This makes the
        // interface more straightforward to use, but internally they're
        // stored in column-major order to avoid having to transpose them
        // when they're uploaded to the GPU.
        Mat4x4(const Float32 *buffer) {
            m00 = buffer[0]; m10 = buffer[1]; m20 = buffer[2]; m30 = buffer[3];
            m01 = buffer[4]; m11 = buffer[5]; m21 = buffer[6]; m31 = buffer[7];
            m02 = buffer[8]; m12 = buffer[9]; m22 = buffer[10]; m32 = buffer[11];
            m03 = buffer[12]; m13 = buffer[13]; m23 = buffer[14]; m33 = buffer[15];
        }

        void transpose(void) {
            swap(m01, m10);
            swap(m02, m20);
            swap(m03, m30);
            swap(m12, m21);
            swap(m13, m31);
            swap(m23, m32);
        }

        static Mat4x4 identity(void) {
            static const Float32 buf[] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
            return Mat4x4(buf);
        }

        struct {
            Float32 m00, m01, m02, m03;
            Float32 m10, m11, m12, m13;
            Float32 m20, m21, m22, m23;
            Float32 m30, m31, m32, m33;
        };
        Float32 m[4][4];
    };

    Mat4x4 operator*(const Mat4x4 &a, const Mat4x4 &b) {
        Mat4x4 m;

        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                m.m[i][j] =
                    a.m[0][j] * b.m[i][0] +
                    a.m[1][j] * b.m[i][1] +
                    a.m[2][j] * b.m[i][2] +
                    a.m[3][j] * b.m[i][3];
            }
        }

        return m;
    }
}

#endif /* KAI_MATH_H */
