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
        T t = min(v0, v1);
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

    inline constexpr Float32 pi = 3.14159265359f;
    inline constexpr Float32 pi2 = 6.28318530718f;

    constexpr Float32 deg_to_rad(Float32 deg) {
        return (deg / 180.0f) * pi;
    }

    constexpr Float32 rad_to_deg(Float32 rad) {
        return (rad / pi) * 180.0f;
    }

    bool nearly_equal(Float32 a, Float32 b) {
        return abs(a - b) <= FLT_EPSILON;
    }

    Float32 square_root(Float32 value) {
        return sqrtf(value);
    }

    Float32 cosine(Float32 angle) {
        return cosf(angle);
    }

    Float32 sine(Float32 angle) {
        return sinf(angle);
    }

    Float32 tangent(Float32 angle) {
        return tanf(angle);
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
        Vec2(void) = default;
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
        Vec4(void) = default;
        Vec4(Float32 x, Float32 y = 0.0f, Float32 z = 0.0f, Float32 w = 0.0f) : x(x), y(y), z(z), w(w) {}

        static Vec4 up(void) {
            return { 0.0f, 1.0f, 0.0f, 0.0f };
        }

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
        Mat4x4(void) = default;

        // Matrices need to be provided in row-major order. This makes the
        // interface more straightforward to use. Internally they're
        // stored in column-major order to avoid having to transpose them
        // before sending them to the GPU.
        Mat4x4(const Float32 (&buffer)[16]) {
            m00 = buffer[0]; m10 = buffer[1]; m20 = buffer[2]; m30 = buffer[3];
            m01 = buffer[4]; m11 = buffer[5]; m21 = buffer[6]; m31 = buffer[7];
            m02 = buffer[8]; m12 = buffer[9]; m22 = buffer[10]; m32 = buffer[11];
            m03 = buffer[12]; m13 = buffer[13]; m23 = buffer[14]; m33 = buffer[15];
        }

        static Mat4x4 identity(void) {
            Mat4x4 m;

            m.m00 = m.m11 = m.m22 = m.m33 = 1.0f;

            return m;
        }

        static Mat4x4 scale(Float32 x = 1.0f, Float32 y = 1.0f, Float32 z = 1.0f) {
            Mat4x4 m;

            m.m00 = x;
            m.m11 = y;
            m.m22 = z;
            m.m33 = 1.0f;

            return m;
        }

        static Mat4x4 translate(Float32 x = 0.0f, Float32 y = 0.0f, Float32 z = 0.0f) {
            Mat4x4 m = identity();

            m.m30 = x;
            m.m31 = y;
            m.m32 = z;

            return m;
        }

        static Mat4x4 rotate_x(Float32 angle) {
            Float32 c = cosine(angle);
            Float32 s = sine(angle);

            Mat4x4 m;

            m.m00 = 1.0f;
            m.m11 = c;
            m.m12 = s;
            m.m21 = -s;
            m.m22 = c;
            m.m33 = 1.0f;

            return m;
        }

        static Mat4x4 rotate_y(Float32 angle) {
            Float32 c = cosine(angle);
            Float32 s = sine(angle);

            Mat4x4 m;

            m.m00 = c;
            m.m02 = -s;
            m.m11 = 1.0f;
            m.m20 = s;
            m.m22 = c;
            m.m33 = 1.0f;

            return m;
        }

        static Mat4x4 rotate_z(Float32 angle) {
            Float32 c = cosine(angle);
            Float32 s = sine(angle);

            Mat4x4 m;

            m.m00 = c;
            m.m01 = s;
            m.m10 = -s;
            m.m11 = c;
            m.m22 = 1.0f;
            m.m33 = 1.0f;

            return m;
        }

        static Mat4x4 look_at_rh(const Vec4 &eye, const Vec4 &target, const Vec4 &up = Vec4::up()) {
            Vec4 f = eye - target;
            f.normalize();

            Vec4 u = up;
            u.normalize();
            Vec4 s = kai::cross(u, f);
            s.normalize();

            u = cross(f, s);

            return Mat4x4({
                s.x,  s.y,  s.z, -kai::dot(s, eye),
                u.x,  u.y,  u.z, -kai::dot(u, eye),
                f.x,  f.y,  f.z, -kai::dot(f, eye),
                0.0f, 0.0f, 0.0f,             1.0f
            });
        }

        static Mat4x4 ortho(Float32 left, Float32 right, Float32 bottom,
                            Float32 top, Float32 near_z, Float32 far_z) {
            Float32 diff_s = right - left;
            Float32 diff_u = top - bottom;
            Float32 diff_f = far_z - near_z;

            return Mat4x4({
                             2.0f / diff_s,                       0.0f,                         0.0f, 0.0f,
                                      0.0f,              2.0f / diff_u,                         0.0f, 0.0f,
                                      0.0f,                       0.0f,               -2.0f / diff_f, 0.0f,
                -((right + left) / diff_s), -((top + bottom) / diff_u), -((far_z + near_z) / diff_f), 1.0f
            });
        }

        static Mat4x4 perspective(Float32 fov, Float32 aspect_ratio, Float32 near_z, Float32 far_z) {
            Float32 t = tangent(fov * 0.5f);

            return Mat4x4({
                1.0f / (aspect_ratio * t),     0.0f,                                0.0f,                                     0.0f,
                                     0.0f, 1.0f / t,                                0.0f,                                     0.0f,
                                     0.0f,     0.0f, (far_z + near_z) / (near_z - far_z), 2.0f * far_z * near_z / (near_z - far_z),
                                     0.0f,     0.0f,                               -1.0f,                                     0.0f
            });
        }

        void transpose(void) {
            swap(m01, m10);
            swap(m02, m20);
            swap(m03, m30);
            swap(m12, m21);
            swap(m13, m31);
            swap(m23, m32);
        }

        struct {
            Float32 m00, m01, m02, m03;
            Float32 m10, m11, m12, m13;
            Float32 m20, m21, m22, m23;
            Float32 m30, m31, m32, m33;
        };
        Float32 m[4][4] = {};
    };

    Mat4x4 operator*(const Mat4x4 &a, const Mat4x4 &b) {
        Mat4x4 m;

        for(Int32 i = 0; i < 4; i++) {
            for(Int32 j = 0; j < 4; j++) {
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
