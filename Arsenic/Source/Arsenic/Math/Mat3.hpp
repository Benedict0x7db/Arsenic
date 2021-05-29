#pragma once

#include "Arsenic/Math/Vec3.hpp"

#include <array>

namespace arsenic
{
namespace math
{
    template<typename T>
    struct Mat3
    {
        using column_type = Vec3<T>;

        std::array<column_type, 3> values;

        Mat3(const T diagonal = static_cast<T>(1))
        {
            values[0][0] = diagonal;
            values[1][1] = diagonal;
            values[2][2] = diagonal;
        }

        column_type &operator[](const std::size_t column) 
        {
            assert(column < 3);            
            return values[column];
        }

        const column_type &operator[](const std::size_t column) const
        {
            assert(column < 3);

            return values[column];
        }
    };

    template<typename T>
    constexpr Mat3<T> operator+(const Mat3<T> &lhs, const Mat3<T> &rhs) noexcept
    {
        Mat3<T> result;
        result[0] = lhs[0] + rhs[0];
        result[1] = lhs[1] + rhs[1];
        result[2] = lhs[2] + rhs[2];

        return result;
    }

    
    template<typename T>
    constexpr Mat3<T> operator-(const Mat3<T> &lhs, const Mat3<T> &rhs) noexcept
    {
        Mat3<T> result;
        result[0] = lhs[0] - rhs[0];
        result[1] = lhs[1] - rhs[1];
        result[2] = lhs[2] - rhs[2];

        return result;
    }

    template<typename T>
    constexpr Mat3<T> operator*(const Mat3<T> &lhs, const Mat3<T> &rhs) noexcept
    {
        Mat3<T> result;
        result[0] = lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2];
        result[1] = lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2];
        result[2] = lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2];

        return result;
    }

    template<typename T>
    constexpr Mat3<T> operator*(const Mat3<T> &mat3, const T scalar) noexcept
    {
        Mat3<T> result(mat3);
        result[0] *= scalar;
        result[1] *= scalar;
        result[2] *= scalar;

        return result;
    }

    template<typename T>
    constexpr Mat3<T> operator*(const T scalar, const Mat3<T> &mat3) noexcept
    {
        Mat3<T> result(mat3);
        result[0] *= scalar;
        result[1] *= scalar;
        result[2] *= scalar;

        return result;
    }

    template<typename T>
    constexpr Vec3<T> operator*(const Mat3<T> &mat3, const Vec3<T> &vec3) noexcept
    {
        return mat3[0] * vec3[0] + mat3[1] * vec3[1] + mat3[2] * vec3[2];
    }

    template<typename T>
    constexpr T determinant(const Mat3<T> &mat3) noexcept
    {
        return math::dot(-math::cross(mat3[0], mat3[2]), mat3[1]);
    }

    template<typename T>
    constexpr Mat3<T> inverse(const Mat3<T> &mat3) noexcept
    {
        const T detEiFh = mat3[0][0] * (mat3[1][1] * mat3[2][2] - mat3[2][1] * mat3[1][2]);
        const T detDiFg = mat3[1][0] * (mat3[0][1] * mat3[2][2] - mat3[2][1] * mat3[0][2]);
        const T detDhEg = mat3[2][0] * (mat3[0][1] * mat3[1][2] - mat3[1][1] * mat3[0][2]);

        const T detMat3 = math::dot(-math::cross(mat3[0], mat3[2]), mat3[1]);

        if (detMat3 == 0) {
            return {};
        }

        // matrix of miniors, cofactors, adjugate
        Mat3<T> temp;
        temp[0][0] = detEiFh;
        temp[0][1] = -detDiFg;
        temp[0][2] = detDhEg;

        temp[1][0] = -(mat3[1][0] * mat3[2][2] - mat3[2][0] * mat3[1][2]);
        temp[1][1] = (mat3[0][0] * mat3[2][2] - mat3[2][0] * mat3[0][2]);
        temp[1][2] = -(mat3[0][0] * mat3[1][2] - mat3[1][2] * mat3[0][2]);

        temp[2][0] = (mat3[1][0] * mat3[2][1] - mat3[2][0] * mat3[1][1]);
        temp[2][1] = -(mat3[0][0] * mat3[2][1] - mat3[2][0] * mat3[0][1]);
        temp[2][2] = (mat3[0][0] * mat3[1][1] - mat3[1][0] * mat3[0][1]);

        return (static_cast<T>(1) / detMat3) * temp;
    }
}
}