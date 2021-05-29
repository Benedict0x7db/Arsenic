#pragma once

#include "Arsenic/Math/Vec2.hpp"
#include "Arsenic/Math/Vec3.hpp"

#include <type_traits>
#include <cassert>
#include <cmath>

namespace arsenic
{
namespace math
{
    template<typename T>
    struct Vec4
    {
        T x;
        T y;
        T z;
        T w;

        constexpr Vec4(const T value = static_cast<T>(0)) noexcept :
            x(value),
            y(value),
            z(value),
            w(value)
        {
        }

        constexpr Vec4(const T x, const T y, const T z, const T w) noexcept :
            x(x),
            y(y),
            z(z),
            w(w)
        {
        }

        constexpr Vec4(const Vec4<T> &vec4) noexcept : 
            x(vec4.x),
            y(vec4.y),
            z(vec4.z),
            w(vec4.w) 
        {
        }

        constexpr Vec4(const Vec3<T> &vec3) noexcept  :
            x(vec3.x),
            y(vec3.y),
            z(vec3.z),
            w(static_cast<T>(0))
        {
        }

        constexpr Vec4(const Vec3<T> &vec3, const T w) noexcept  :
            x(vec3.x),
            y(vec3.y),
            z(vec3.z),
            w(w)
        {
        }

        Vec4(const Vec2<T> &vec2) noexcept :
            x(vec2.x),
            y(vec2.y),
            z(static_cast<T>(0)),
            w(static_cast<T>(0))
        {
        }

        Vec4(const Vec2<T> &vec2, const T z, const T w) noexcept :
            x(vec2.x),
            y(vec2.y),
            z(z),
            w(w)
        {
        }

        T& operator[](const std::size_t n) noexcept
        {
            assert(n < 4);
            return *(&x + n);
        }

        
        const T& operator[](const std::size_t n) const noexcept
        {
            assert(n < 4);
            return *(&x + n);
        }

        const Vec4<T> &operator+=(const Vec4<T> &vec4) noexcept
        {
            x += vec4.x;
            y += vec4.y;
            z += vec4.z;
            w += vec4.w;

            return *this;
        }

        Vec4<T> &operator+=(const T scalar) noexcept
        {
            x += scalar;
            y += scalar;
            z += scalar;
            w += scalar;

            return *this;
        }

        Vec4<T> &operator-=(const Vec4<T> &vec4) noexcept
        {
            x -= vec4.x;
            y -= vec4.y;
            z -= vec4.z;
            w -= vec4.w;

            return *this;
        }

        Vec4<T> &operator-=(const T scalar) noexcept
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            w -= scalar;

            return *this;
        }

        Vec4<T> &operator*=(const T scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;

            return *this;
        }

        const Vec4<T> &operator/=(const T scalar) noexcept
        {
            static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
            assert(scalar);

            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;

            return *this;
        }
    };

    template<typename T>
    constexpr Vec4<T> operator+(const Vec4<T> &lhs, const Vec4<T> &rhs) noexcept
    {
        return Vec4<T>(
            lhs.x + rhs.x, 
            lhs.y + rhs.y,
            lhs.z + rhs.z,
            lhs.w + rhs.w
        );
    }

    template<typename T>
    constexpr Vec4<T> operator+(const Vec4<T> &vec4, const T scalar)
    {
        return Vec4<T>{
            vec4.x + scalar,
            vec4.y + scalar,
            vec4.z + scalar,
            vec4.w + scalar
        };
    }

    template<typename T>
    constexpr Vec4<T> operator+(const T scalar, const Vec4<T> &vec4)
    {
        return Vec4<T>{
            vec4.x + scalar,
            vec4.y + scalar,
            vec4.z + scalar,
            vec4.w + scalar
        };
    }

    template<typename T>
    constexpr Vec4<T> operator-(const Vec4<T> &lhs, const Vec4<T> &rhs) noexcept
    {
        return Vec4<T>{
            lhs.x - rhs.x, 
            lhs.y - rhs.y,
            lhs.z - rhs.z,
            lhs.w - rhs.w
        };
    }

    template<typename T>
    constexpr Vec4<T> operator-(const Vec4<T> &vec4) noexcept
    {
        return Vec4<T>(-vec4.x, -vec4.y, -vec4.z, -vec4.w);
    }

    template<typename T>
    constexpr Vec4<T> operator-(const Vec4<T> &vec4, const T scalar) noexcept
    {
        return Vec4<T>{
            vec4.x - scalar,
            vec4.y - scalar,
            vec4.z - scalar,
            vec4.w - scalar
        };
    }

    template<typename T>
    constexpr Vec4<T> operator-(const T scalar, const Vec4<T> &vec4) noexcept
    {
        return Vec4<T>{
            vec4.x - scalar,
            vec4.y - scalar,
            vec4.z - scalar,
            vec4.w - scalar
        };
    }

    template<typename T>
    constexpr Vec4<T> operator*(const Vec4<T> &vec4, const T scalar) noexcept
    {
        return Vec4<T>(
            vec4.x * scalar,
            vec4.y * scalar,
            vec4.z * scalar,
            vec4.w * scalar
        );
    }

    template<typename T>
    constexpr Vec4<T> operator*(const T scalar, const Vec4<T> &vec4) noexcept
    {
        return Vec4<T>(
            vec4.x * scalar,
            vec4.y * scalar,
            vec4.z * scalar,
            vec4.w * scalar
        );
    }

    template<typename T>
    constexpr Vec4<T> operator/(const Vec4<T> &vec4, const T scalar)
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
        assert(scalar);

        return  Vec4<T>(
            vec4.x / scalar,
            vec4.y / scalar,
            vec4.z / scalar,
            vec4.w / scalar
        );
    }

    template<typename T>
    constexpr Vec4<T> operator/(const T scalar, const Vec4<T> &vec4)
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
        assert(scalar);

        return  Vec4<T>(
            vec4.x / scalar,
            vec4.y / scalar,
            vec4.z / scalar,
            vec4.w / scalar
        );
    }

    template<typename T>
    constexpr T dot(const Vec4<T> &lhs, const Vec4<T> &rhs) noexcept
    {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);
    }

    template<typename T>
    constexpr T length(const Vec4<T> &vec4) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating pint");
        return std::sqrt(dot(vec4, vec4));
    }

    template<typename T>
    constexpr Vec4<T> normalize(const Vec4<T> &vec4) noexcept
    {
        T mag = length(vec4);

        if (mag == static_cast<T>(0)) {
            mag = static_cast<T>(1);
        }

        return Vec4<T>(
            vec4.x / mag,
            vec4.y / mag,
            vec4.z / mag,
            vec4.w / mag
        );
    }

    template<typename T>
    constexpr Vec4<T> project(const Vec4<T> &first, const Vec4<T> &second) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        T mag = math::length(second);

        if (mag == static_cast<T>(0)) {
            mag = static_cast<T>(1);
        }
        
        return second * (math::dot(first, second) / (mag * mag));
    }

    template<typename T>
    constexpr Vec3<T> toVec3(const Vec4<T> &vec4) noexcept 
    {
        return Vec3<T>(vec4.x, vec4.y, vec4.z);
    }

    template<typename T>
    constexpr Vec2<T> toVec2(const Vec4<T> &vec4) noexcept 
    {
        return Vec2<T>(vec4.x, vec4.y);
    }
}
}