#pragma once

#include "Arsenic/Math/Math.hpp"

namespace arsenic 
{
    struct Event;

    struct Camera
    {
        math::vec3f position;
        math::vec3f rotation;
        math::vec3f forwardDir;
        math::mat4f viewMatrix;
        math::mat4f projMatrix;
        float cameraSpeed = 1.0f;
        float sensitivity = 0.1f;
        float aspect = 16.0f / 9.0f;
        float fov = 60.0f;
        float znear = 0.1f;
        float zfar = 100.0f;
        math::vec2f lastMousePos;
        Mousecode rotationButton = Mousecode::ButtonRight;
        bool isRotateButtonPressed = false;

        void initialize(const uint32_t width, const uint32_t height);
        void updateViewMatrix();
        void updateProjMatrix();
        void update(const float dt);
        void onEvent(Event &event);
    };
}