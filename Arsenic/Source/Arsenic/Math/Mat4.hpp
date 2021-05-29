#pragma once

#include "Arsenic/Math/Vec4.hpp"

#include <array>

namespace arsenic
{
namespace math
{
    template<typename T>
    struct Mat4
    {
        using column_type = Vec4<T>;

        std::array<column_type, 4> values;

        Mat4(const T diagonal = static_cast<T>(1))
        {
            values[0][0] = diagonal;
            values[1][1] = diagonal;
            values[2][2] = diagonal;
            values[3][3] = diagonal;   
        }

        column_type &operator[](const std::size_t column) 
        {
            assert(column < 4);            
            return values[column];
        }
 
        const column_type &operator[](const std::size_t column) const
        {
            assert(column < 4);

            return values[column];
        }
    };


    template<typename T>
    constexpr Mat4<T> operator+(const Mat4<T> &lhs, const Mat4<T> &rhs) noexcept
    {
        Mat4<T> result;
        result[0] = lhs[0] + rhs[0];
        result[1] = lhs[1] + rhs[1];
        result[2] = lhs[2] + rhs[2];
        result[3] = lhs[3] + rhs[3];

        return result;
    }

    
    template<typename T>
    constexpr Mat4<T> operator-(const Mat4<T> &lhs, const Mat4<T> &rhs) noexcept
    {
        Mat4<T> result;
        result[0] = lhs[0] - rhs[0];
        result[1] = lhs[1] - rhs[1];
        result[2] = lhs[2] - rhs[2];
        result[3] = lhs[3] - rhs[3];

        return result;
    }

    template<typename T>
    constexpr Mat4<T> operator*(const Mat4<T> &lhs, const Mat4<T> &rhs) noexcept
    {
        Mat4<T> result;
        result[0] = lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2] + lhs[3] * rhs[0][3];
        result[1] = lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2] + lhs[3] * rhs[1][3];
        result[2] = lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2] + lhs[3] * rhs[2][3];
        result[3] = lhs[0] * rhs[3][0] + lhs[1] * rhs[3][1] + lhs[2] * rhs[3][2] + lhs[3] * rhs[3][3];

        return result;
    }

    template<typename T>
    constexpr Mat4<T> operator*(const Mat4<T> &mat4, const T scalar) noexcept
    {
        Mat4<T> result(mat4);
        result[0] *= scalar;
        result[1] *= scalar;
        result[2] *= scalar;
        result[3] *= scalar;

        return result;
    }

    template<typename T>
    constexpr Mat4<T> operator*(const T scalar, const Mat4<T> &mat4) noexcept
    {
        Mat4<T> result(mat4);
        result[0] *= scalar;
        result[1] *= scalar;
        result[2] *= scalar;
        result[3] *= scalar;

        return result;
    }

    template<typename T>
    constexpr Vec4<T> operator*(const Mat4<T> &Mat4, const Vec4<T> &vec4) noexcept
    {
        return Mat4[0] * vec4[0] + Mat4[1] * vec4[1] + Mat4[2] * vec4[2] + Mat4[3] * vec4[3];
    }

     
    template<typename T>
    constexpr T determinant(const Mat4<T> &mat4) noexcept
    {
        const T detKpLo = mat4[2][2] * mat4[3][3] - mat4[3][2] * mat4[2][3];
        const T detJpLn = mat4[1][2] * mat4[3][3] - mat4[3][2] * mat4[1][3];
        const T detJoKn = mat4[1][2] * mat4[2][3] - mat4[2][2] * mat4[1][3];
        const T detIpLm = mat4[0][2] * mat4[3][3] - mat4[3][2] * mat4[0][3];
        const T detIoKm = mat4[0][2] * mat4[2][3] - mat4[2][2] * mat4[0][3];
        const T detInJm = mat4[0][2] * mat4[1][3] - mat4[1][2] * mat4[0][3];

       const T result = mat4[0][0] * (mat4[1][1] * detKpLo - mat4[2][1] * detJpLn + mat4[3][1] * detJoKn) -
                        mat4[1][0] * (mat4[0][1] * detKpLo - mat4[2][1] * detIpLm + mat4[3][1] * detIoKm) +
                        mat4[2][0] * (mat4[0][1] * detJpLn - mat4[1][1] * detIpLm + mat4[3][1] * detInJm) -
                        mat4[3][0] * (mat4[0][1] * detJoKn - mat4[1][1] * detIoKm + mat4[2][1] * detInJm);

        return result;
    }

    template<typename T>
    constexpr Mat4<T> inverse(const Mat4<T> &mat4) noexcept
    {
        const T detKpLo = mat4[2][2] * mat4[3][3] - mat4[3][2] * mat4[2][3];
        const T detJpLn = mat4[1][2] * mat4[3][3] - mat4[3][2] * mat4[1][3];
        const T detJoKn = mat4[1][2] * mat4[2][3] - mat4[2][2] * mat4[1][3];
        const T detIpLm = mat4[0][2] * mat4[3][3] - mat4[3][2] * mat4[0][3];
        const T detIoKm = mat4[0][2] * mat4[2][3] - mat4[2][2] * mat4[0][3];
        const T detInJm = mat4[0][2] * mat4[1][3] - mat4[1][2] * mat4[0][3];
        const T detFpHn = mat4[1][1] * mat4[3][3] - mat4[3][1] * mat4[1][3];
        const T detEpHm = mat4[0][1] * mat4[3][3] - mat4[3][1] * mat4[0][3];
        const T detEnFm = mat4[0][1] * mat4[1][3] - mat4[1][1] * mat4[0][3];
        const T detGpHo = mat4[2][1] * mat4[3][3] - mat4[3][1] * mat4[2][3];
        const T detFoGn = mat4[1][1] * mat4[1][3] - mat4[2][1] * mat4[1][3];
        const T detEoGm = mat4[0][1] * mat4[1][3] - mat4[2][1] * mat4[0][3];
        const T detGlHk = mat4[2][1] * mat4[3][2] - mat4[3][1] * mat4[2][2];
        const T detFlHj = mat4[1][1] * mat4[3][2] - mat4[3][1] * mat4[1][2];
        const T detFkGl = mat4[1][1] * mat4[2][2] - mat4[2][1] * mat4[3][2];
        const T detEkGi = mat4[0][1] * mat4[2][2] - mat4[2][1] * mat4[0][2];
        const T detFkGj = mat4[1][1] * mat4[2][2] - mat4[2][1] * mat4[1][2];
        const T detEjFi = mat4[0][1] * mat4[1][2] - mat4[1][1] * mat4[0][2];
        const T detElHi = mat4[0][1] * mat4[3][2] - mat4[3][1] * mat4[0][2];        
        
        const T detMat4 = mat4[0][0] * (mat4[1][1] * detKpLo - mat4[2][1] * detJpLn + mat4[3][1] * detJoKn) -
                          mat4[1][0] * (mat4[0][1] * detKpLo - mat4[2][1] * detIpLm + mat4[3][1] * detIoKm) +
                          mat4[2][0] * (mat4[0][1] * detJpLn - mat4[1][1] * detIpLm + mat4[3][1] * detInJm) -
                          mat4[3][0] * (mat4[0][1] * detJoKn - mat4[1][1] * detIoKm + mat4[2][1] * detInJm);

        if (!detMat4) {
            return {};
        }

        // matrix of minior, cofactors, and adjugate
        Mat4<T> temp(static_cast<T>(0));
        temp[0][0] = ( mat4[1][1] * detKpLo - mat4[2][1] * detJpLn + mat4[3][1] * detJoKn  );
        temp[0][1] = -(  mat4[0][1] * detKpLo - mat4[2][1] * detIpLm + mat4[3][1] * detIoKm  );
        temp[0][2] = ( mat4[0][1] * detJpLn - mat4[1][1] * detIpLm + mat4[3][1] * detInJm  );
        temp[0][3] = -(  mat4[0][1] * detJoKn - mat4[1][1] * detIoKm + mat4[2][1] * detInJm  );

        temp[1][0] = -(  mat4[1][0] * detKpLo - mat4[2][0] * detJpLn + mat4[3][0] * detJoKn  );
        temp[1][1] = ( mat4[0][0] * detKpLo - mat4[2][0] * detIpLm + mat4[3][0] * detIoKm  );
        temp[1][2] = -(  mat4[0][0] * detJpLn - mat4[1][0] * detIpLm + mat4[3][0] * detInJm  );
        temp[1][3] = ( mat4[0][0] * detJoKn - mat4[1][0] * detIoKm + mat4[2][0] * detInJm  );

        temp[2][0] = ( mat4[1][0] * detGpHo - mat4[2][0] * detFpHn + mat4[3][0] * detFoGn  );
        temp[2][1] = -(  mat4[0][0] * detGpHo - mat4[2][0] * detEpHm + mat4[3][0] * detEoGm  );
        temp[2][2] = ( mat4[0][0] * detFpHn - mat4[1][0] * detEpHm + mat4[3][0] * detEnFm  );
        temp[2][3] = -(  mat4[0][0] * detFoGn - mat4[1][0] * detEoGm + mat4[2][0] * detEnFm  );

        temp[3][0] = -(  mat4[1][0] * detGlHk - mat4[2][0] * detFlHj + mat4[3][0] * detFkGj  );
        temp[3][1] = ( mat4[0][0] * detGlHk - mat4[2][0] * detElHi + mat4[3][0] * detEkGi  );
        temp[3][2] = -(  mat4[0][0] * detFlHj - mat4[1][0] * detElHi + mat4[3][0] * detEjFi  );
        temp[3][3] = ( mat4[0][0] * detFkGj - mat4[1][0] * detEkGi + mat4[2][0] * detEjFi  );

        return (static_cast<T>(1) / detMat4) * temp;
    }
}
}