#pragma once

#include "Arsenic/Math/MathFunctions.hpp"
#include "Arsenic/Math/Vec2.hpp"
#include "Arsenic/Math/Vec3.hpp"
#include "Arsenic/Math/Vec4.hpp"
#include "Arsenic/Math/Mat4.hpp"
#include "Arsenic/Math/Mat3.hpp"
#include "Arsenic/Math/ClipSpace.hpp"
#include "Arsenic/Math/MatrixConvert.hpp"
#include "Arsenic/Math/MatrixTransforms.hpp"

namespace arsenic
{
namespace math
{
    using vec4f = Vec4<float>;
    using vec4d = Vec4<double>;
    using vec4i = Vec4<int>;
    using vec4ui = Vec4<unsigned int>;

    using vec3f = Vec3<float>;
    using vec3d = Vec3<double>;
    using vec3i = Vec3<int>;
    using vec3ui = Vec3<unsigned int>;

    using vec2f = Vec2<float>;
    using vec2d = Vec2<double>;
    using vec2i = Vec2<int>;
    using vec2ui = Vec2<unsigned int>;

    using mat4f = Mat4<float>;
    using mat4d = Mat4<double>;
    using mat4i = Mat4<int>;
    using mat4ui = Mat4<unsigned int>;    

    using mat3f = Mat3<float>;
    using mat3d = Mat3<double>;
    using mat3i = Mat3<int>;
    using mat3ui = Mat3<unsigned int>;     
}
}