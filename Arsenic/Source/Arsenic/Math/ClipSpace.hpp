#pragma once

#include "Arsenic/Math/Mat4.hpp"
#include "Arsenic/Math/Vec4.hpp"
#include "Arsenic/Math/MathFunctions.hpp"

#include <type_traits>

namespace arsenic
{
namespace math
{
    template<typename T = float>
    constexpr Mat4<T> createOrtho(const T left, const T right, const T bottom, const T top, const T near = 0.0, const T far = 1.0) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        Mat4<T> result(static_cast<T>(0));
        result[0][0] = static_cast<T>(2) / (right - left);
        result[1][1] = static_cast<T>(2) / (top - bottom);
        result[2][2] = static_cast<T>(1) / (far - near);
        result[3][0] = -(right + left) / (right - left);
        result[3][1] = -(top + bottom) / (top - bottom);
        result[3][2] = -near / (far - near);
        result[3][3] = 1;
        
        return result;
    }
    
    template<typename T = float>
    constexpr Mat4<T> createFrustum(const T left, const T right, const T bottom, const T top, const T znear, const T zfar) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        Mat4<T> result(static_cast<T>(0));
        result[0][0] = -(2 * znear) / (right - left);
        result[1][1] = -(2 * znear) / (top - bottom);
        result[2][2] = -znear / (zfar - znear);
        result[2][0] = -(right + left) / (right - left);
        result[2][1] = -(top + bottom) / (top - bottom);
        result[3][2] = -(znear * zfar) / (zfar - znear);
        result[2][3] = static_cast<T>(1);

        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> createPerspective(const float aspect, const float fov, const float znear, const float zfar) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T halfTan = std::tan(fov * 0.5f);

        Mat4<T> result(static_cast<T>(0));
        result[0][0] = -static_cast<T>(1) / halfTan;
        result[1][1] = -aspect / halfTan;
        result[2][2] = zfar / (zfar - znear);
        result[3][2] = -(znear * zfar)/ (zfar - znear);
        result[2][3] = static_cast<T>(1);

        return result;
    }

    template<typename T = float>
    constexpr Mat4<T> createPerspectiveDepthOneZero(const float aspect, const float fov, const float znear, const float zfar) noexcept
    {
        static_assert(std::is_floating_point_v<T>, "T has to be a floating point");

        const T halfTan = std::tan(fov * 0.5f);
        const T left = halfTan * -znear;
        const T right = -left;
        const T bottom = left / aspect;
        const T top = -bottom;

        return createFrustum(left, right, bottom, top, znear, zfar);
    }
}
}