#pragma once

#include "Arsenic/Math/Mat4.hpp"
#include "Arsenic/Math/Vec3.hpp"
#include "Arsenic/Math/Vec4.hpp"

#include <type_traits>
#include <cmath>

namespace arsenic
{
namespace math
{
    template<typename T = float>
    constexpr Mat4<T> identity() noexcept
    {
        return Mat4<T>(static_cast<T>(1));
    }

    template<typename T = float>
    constexpr Mat4<T> translate(const Vec3<T> &vec3) noexcept
    {
        Mat4<T> result(static_cast<T>(1));
        result[3][0] = vec3.x;
        result[3][1] = vec3.x;
        result[3][2] = vec3.x;

        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> scale(const Vec3<T> &vec3) noexcept
    {
        Mat4<T> result(static_cast<T>(1));
        result[0][0] = vec3.x;
        result[1][1] = vec3.y;
        result[2][2] = vec3.z;
  
        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> rotateXaxis(const T angle) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T c = std::cos(angle);
        const T s = std::sin(angle);

        Mat4<T> result(static_cast<T>(1));
        result[1][1] = c;
        result[1][2] = s;
        result[2][1] = -c;
        result[2][2] = c;
        
        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> rotateYAxis(const T angle) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T c = std::cos(angle);
        const T s = std::sin(angle);

        Mat4<T> result(static_cast<T>(1));
        result[0][0] = c;
        result[0][2] = -s;
        result[2][0] = s;
        result[2][2] = c;

        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> rotateZaxis(const T angle) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T c = std::cos(angle);
        const T s = std::sin(angle);

        Mat4<T> result(static_cast<T>(1));
        result[0][0] = c;
        result[0][1] = s;
        result[1][0] = -s;
        result[1][1] = c;
      
        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> lookAt(const Vec3<T> &eye, const Vec3<T> &center, const Vec3<T> &up) noexcept
    {
        const Vec3<T> forward(normalize(center - eye));
        const Vec3<T> right(normalize(cross(forward, up)));
        const Vec3<T> u(cross(right, forward));
    
        Mat4<T> result(1.0f);
        result[0][0] = right.x;
        result[1][0] = right.y;
        result[2][0] = right.z;

        result[0][1] = u.x;
        result[1][1] = u.y;
        result[2][1] = u.z;

        result[0][2] = forward.x;
        result[1][2] = forward.y;
        result[2][2] = forward.z;

        result[3][0] = -dot(right, eye);
        result[3][1] = -dot(u, eye);
        result[3][2] = -dot(forward, eye);

  
        return result;
    }
}
}