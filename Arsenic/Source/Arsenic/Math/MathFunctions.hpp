#pragma once

#include "Arsenic/Math/Vec2.hpp"
#include "Arsenic/Math/Vec4.hpp"

#include <type_traits>

namespace arsenic
{
namespace math
{
    constexpr double pi = 3.141592653589793238462643383279502884L;

    template<typename T>
    constexpr T radians(const T deg) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        return static_cast<T>(pi) * deg / static_cast<T>(180);
    }

    template<typename T>
    constexpr T degrees(const T rad) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        return rad * static_cast<T>(180) / static_cast<T>(pi);
    }
    
    template<typename T>
    constexpr T lerp(const T a, const T b, const T t) noexcept
    {
        return a * (static_cast<T>(1) - t) + (b * t);
    };

    template<typename T>
    constexpr T bilerp(const Vec2<T> v0, const Vec2<T> v1, const T t0, const T t1) 
    {
        T a = lerp(v0.x, v0.y, t0);
        T b = lerp(v1.x, v1.y, t0);

        return lerp(a, b, t1);
    }

    template<typename T>
    constexpr T trilerp(const Vec4<T> &v0, const Vec4<T> &v1, const T t0, const T t1, const T t2)
    {
        T a = bilerp(Vec2<T>(v0.x, v0.y), Vec2<T>(v0.z, v0.w), t0, t1);
        T b = bilerp(Vec2<T>(v1.x, v1.y), Vec2<T>(v1.z, v1.w), t0, t1);

        return lerp(a, b, t2);
    }
}
}