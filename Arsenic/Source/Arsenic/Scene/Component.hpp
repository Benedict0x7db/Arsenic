#pragma once

#include "Arsenic/Math/Math.hpp"

namespace arsenic
{  
    struct Transform
    {
        math::vec3f position;
        math::vec3f rotation;
        math::vec3f scale = math::vec3f(1.0f, 1.0f, 1.0f);
       
        Transform() = default;
        Transform(const math::vec3f position) : position(position) {}

        math::mat4f getModelMatrix() const
        {
            math::mat4f rotations = math::rotateYAxis(math::radians(rotation.x)) * math::rotateZaxis(math::radians(rotation.z)) * 
                                    math::rotateYAxis(math::radians(rotation.y));
        
            return math::translate(position) * rotations * math::scale(scale); 
        }
    };

    struct PointLight
    {
        math::vec3f color = math::vec3f(1.0f);
        float intensity = 1.0f; 

        PointLight() = default;

        PointLight(const math::vec3f color, const float intensity) :
            color(color),
            intensity(intensity)
        {
        }     
         
    };

    struct DirectionLight
    {
        math::vec3f color = math::vec3f(1.0f);
        float intensity = 1.0f;     
    };

    struct SpotLight
    {
        math::vec3f color = math::vec3f(1.0f);
        float intensity = 1.0f;
        math::vec3f position;
        float innerCutoff = 15.0f;
        math::vec3f direction;
        float outerCutoff = 20.0f;
    };
}