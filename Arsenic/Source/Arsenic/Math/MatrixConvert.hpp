#pragma once

#include "Arsenic/Math/Mat3.hpp"
#include "Arsenic/Math/Mat4.hpp"

namespace arsenic
{
namespace math
{
    template<typename T>
    constexpr Mat4<T> toMat4(const Mat3<T> &mat3) noexcept
    {
        Mat4<T> result;
        result[0] = Vec4<T>(mat3[0]);
        result[1] = Vec4<T>(mat3[1]);
        result[2] = Vec4<T>(mat3[2]);
        result[3][3] = static_cast<T>(1);

        return result;
    }


    template<typename T>
    constexpr Mat3<T> toMat3(const Mat4<T> &mat4) noexcept
    {
        Mat3<T> result;
        result[0] = toVec3(mat4[0]);
        result[1] = toVec3(mat4[1]);
        result[2] = toVec3(mat4[2]);

        return result;
    }
}
}