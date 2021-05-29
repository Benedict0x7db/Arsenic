#pragma once

#include <type_traits>
#include <cassert>
#include <cmath>

namespace arsenic
{
namespace math
{
    template<typename T>
    struct Vec2
    {
        T x;
        T y;

        constexpr Vec2(const T value = static_cast<T>(0)) noexcept :
            x(value),
            y(value)
        {
        }

        constexpr Vec2(const T x, const T y) :
            x(x),
            y(y)
        {
        }

        constexpr Vec2(const Vec2<T> &vec2) noexcept : 
            x(vec2.x),
            y(vec2.y)
        {
        }

        T& operator[](const std::size_t n) noexcept
        {
            assert(n < 2);
            return *(&x + n);
        }
        
        const T& operator[](const std::size_t n) const noexcept
        {
            assert(n < 2);
            return *(&x + n);
        }

        const Vec2<T> &operator+=(const Vec2<T> &vec2) noexcept
        {
            x += vec2.x;
            y += vec2.y;

            return *this;
        }

        const Vec2<T> &operator-=(const Vec2<T> &vec2) noexcept
        {
            x -= vec2.x;
            y -= vec2.y;

            return *this;
        }

        const Vec2<T> &operator+=(const T scalar) noexcept
        {
            x += scalar;
            y += scalar;

            return *this;
        }

        const Vec2<T> &operator-=(const T scalar) noexcept
        {
            x -= scalar;
            y -= scalar;

            return *this;
        }

        const Vec2<T> &operator*=(const T scalar)
        {
            x *= scalar;
            y *= scalar;

            return *this;
        }

        const Vec2<T> &operator/=(const T scalar)
        {
            static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
            assert(scalar);

            x /= scalar;
            y /= scalar;

            return *this;
        }
    };

    template<typename T>
    constexpr Vec2<T> operator-(const Vec2<T> &vec2) noexcept
    {
        return Vec2<T>(-vec2.x, -vec2.y);
    }

    template<typename T>
    constexpr Vec2<T> operator+(const Vec2<T> &lhs, const Vec2<T> &rhs) noexcept
    {
        return Vec2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    template<typename T>
    constexpr Vec2<T> operator+(const Vec2<T> &vec2, const T scalar) noexcept
    {
        return Vec2<T>(vec2.x + scalar, vec2.y + scalar);
    }

    template<typename T>
    constexpr Vec2<T> operator-(const Vec2<T> &lhs, const Vec2<T> &rhs) noexcept
    {
        return Vec2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    template<typename T>
    constexpr Vec2<T> operator-(const Vec2<T> &vec2, const T scalar)
    {
        return Vec2<T>(vec2.x - scalar, vec2.y - scalar);
    }

    template<typename T>
    constexpr Vec2<T> operator-(const T scalar, const Vec2<T> &vec2)
    {
        return Vec2<T>(vec2.x - scalar, vec2.y - scalar);
    }

    template<typename T>
    constexpr Vec2<T> operator*(const Vec2<T> &vec2, const T scalar)
    {
        return Vec2<T>(vec2.x * scalar, vec2.y * scalar);
    }

    template<typename T>
    constexpr Vec2<T> operator*(const T scalar, const Vec2<T> &vec2)
    {
        return {
            vec2.x * scalar,
            vec2.y * scalar
        };
    }

    template<typename T>
    constexpr Vec2<T> operator/(const Vec2<T> &vec2, const T scalar)
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
        assert(scalar);

        return Vec2<T>(vec2.x / scalar, vec2.y / scalar);
    }

    template<typename T>
    constexpr Vec2<T> operator/(const T scalar, const Vec2<T> &vec2)
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");
        assert(scalar);

        return Vec2<T>(vec2.x / scalar, vec2.y / scalar);
    }


    template<typename T>
    constexpr T dot(const Vec2<T> &lhs, const Vec2<T> &rhs) noexcept
    {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y);
    }

    template<typename T>
    constexpr T length(const Vec2<T> &vec2) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        return std::sqrt(dot(vec2, vec2));
    }

    template<typename T>
    constexpr Vec2<T> normalize(const Vec2<T> &vec2) noexcept
    {
        const T mag = length(vec2);
        return Vec2<T>(vec2.x / mag, vec2.y / mag);
    }

    template<typename T>
    constexpr Vec2<T> project(const Vec2<T> &first, const Vec2<T> &second) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T mag = math::length(second);

        if (mag == 0) {
            return {};
        }
        
        return second * (math::dot(first, second) / (mag * mag));
    }
}
}