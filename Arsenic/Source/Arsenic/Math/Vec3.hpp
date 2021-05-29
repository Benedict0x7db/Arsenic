#pragma once

#include "Arsenic/Math/Vec2.hpp"

#include <type_traits>
#include <cassert>
#include <cmath>

namespace arsenic
{
namespace math
{
    template<typename T>
    struct Vec3
    {
        T x;
        T y;
        T z;

        constexpr Vec3(const T value = static_cast<T>(0)) noexcept :
            x(value),
            y(value),
            z(value)
        {
        }

        constexpr Vec3(const T x, const T y, const T z) :
            x(x),
            y(y),
            z(z)
        {
        }

        constexpr Vec3(const Vec3<T> &vec3) noexcept : 
            x(vec3.x),
            y(vec3.y),
            z(vec3.z)
        {
        }

        constexpr Vec3(const Vec2<T> &vec2) noexcept : 
            x(vec2.x),
            y(vec2.y),
            z(0.0)
        {
        }

        constexpr Vec3(const Vec2<T> &vec2, const T z) noexcept : 
            x(vec2.x),
            y(vec2.y),
            z(z)
        {
        }
        
        T& operator[](const std::size_t n) noexcept
        {
            assert(n < 3);
            return *(&x + n);
        }

        
        const T& operator[](const std::size_t n) const noexcept
        {
            assert(n < 3);
            return *(&x + n);
        }

        const Vec3<T> &operator+=(const Vec3<T> &vec3) noexcept
        {
            x += vec3.x;
            y += vec3.y;
            z += vec3.z;

            return *this;
        }

        const Vec3<T> &operator-=(const Vec3<T> &vec3) noexcept
        {
            x -= vec3.x;
            y -= vec3.y;
            z -= vec3.z;

            return *this;
        }

        const Vec3<T> &operator+=(const T scalar) noexcept
        {
            x += scalar;
            y += scalar;
            z += scalar;

            return *this;
        }

        const Vec3<T> &operator-=(const T scalar) noexcept
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;

            return *this;
        }

        const Vec3<T> &operator*=(const T scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;

            return *this;
        }

        const Vec3<T> &operator/=(const T scalar)
        {
            static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
            assert(scalar);

            x /= scalar;
            y /= scalar;
            z /= scalar;

            return *this;
        }
    };

    template<typename T>
    constexpr Vec3<T> operator+(const Vec3<T> &lhs, const Vec3<T> &rhs) noexcept
    {
        return {
            lhs.x + rhs.x, 
            lhs.y + rhs.y,
            lhs.z + rhs.z
        };
    }

    template<typename T>
    constexpr Vec3<T> operator-(const Vec3<T> &lhs, const Vec3<T> &rhs) noexcept
    {
        return {
            lhs.x - rhs.x, 
            lhs.y - rhs.y,
            lhs.z - rhs.z
        };
    }

    template<typename T>
    constexpr Vec3<T> operator-(const Vec3<T> &vec3) noexcept
    {
        return Vec3<T>(-vec3.x, -vec3.y, -vec3.z);
    }

    template<typename T>
    constexpr Vec3<T> operator+(const Vec3<T> &vec3, const T scalar)
    {
        return {
            vec3.x + scalar,
            vec3.y + scalar,
            vec3.z + scalar
        };
    }

    template<typename T>
    constexpr Vec3<T> operator+(const T scalar, const Vec3<T> &vec3) 
    {
        return {
            vec3.x + scalar,
            vec3.y + scalar,
            vec3.z + scalar
        };   
    }

    template<typename T>
    constexpr Vec3<T> operator-(const Vec3<T> &vec3, const T scalar)
    {
        return {
            vec3.x - scalar,
            vec3.y - scalar,
            vec3.z - scalar
        };
    }

    template<typename T>
    constexpr Vec3<T> operator-(const T scalar, const Vec3<T> &vec3)
    {
        return {
            vec3.x - scalar,
            vec3.y - scalar,
            vec3.z - scalar
        };
    }


    template<typename T>
    constexpr Vec3<T> operator*(const Vec3<T> &vec3, const T scalar)
    {
        return {
            vec3.x * scalar,
            vec3.y * scalar,
            vec3.z * scalar
        };
    }

    template<typename T>
    constexpr Vec3<T> operator*(const T scalar, const Vec3<T> &vec3)
    {
        return {
            vec3.x * scalar,
            vec3.y * scalar,
            vec3.z * scalar
        };
    }

    template<typename T>
    constexpr Vec3<T> operator/(const Vec3<T> &vec3, const T scalar)
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
        assert(scalar);

        return {
            vec3.x / scalar,
            vec3.y / scalar,
            vec3.z / scalar
        };
    }

    template<typename T>
    constexpr T dot(const Vec3<T> &lhs, const Vec3<T> &rhs) noexcept
    {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
    }

    template<typename T>
    constexpr T length(const Vec3<T> &vec3) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating pint");
        
        return std::sqrt(dot(vec3, vec3));
    }

    template<typename T>
    constexpr Vec3<T> normalize(const Vec3<T> &vec3) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating pint");

        const T mag = length(vec3);

        if (mag == 0) {
            return {};
        }
        
        return {
            vec3.x / mag,
            vec3.y / mag,
            vec3.z / mag
        };
    }

    template<typename T>
    constexpr Vec3<T> cross(const Vec3<T> &lhs, const Vec3<T> &rhs) noexcept
    {
        return {
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x
        };  
    }

    template<typename T>
    constexpr Vec3<T> project(const Vec3<T> &first, const Vec3<T> &second) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T mag = math::length(second);

        if (mag == 0) {
            return {};
        }
        
        return second * (math::dot(first, second) / (mag * mag));
    }

    template<typename T>
    constexpr Vec2<T> toVec2(const Vec3<T> &vec3) noexcept 
    {
        return Vec2<T>(vec3.x, vec3.y);
    }
};
}