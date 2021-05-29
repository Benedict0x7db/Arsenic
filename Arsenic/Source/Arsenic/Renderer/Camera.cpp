#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Core/Input.hpp"
#include "Arsenic/Event/Event.hpp"
#include "Arsenic/Event/ApplicationEvents.hpp"
#include "Arsenic/Event/MouseEvents.hpp"
#include "Arsenic/Core/Application.hpp"
#include "Arsenic/Renderer/Camera.hpp"

#include "GLFW/glfw3.h"

namespace arsenic
{
    void Camera::initialize(const uint32_t width, const uint32_t height)
    {
        aspect = static_cast<float>(width) / static_cast<float>(height);
        forwardDir = math::vec3f(0.0f, 0.0f, -1.0f);
        projMatrix = math::createPerspective(aspect, math::radians(fov), znear, zfar);
        viewMatrix = math::lookAt(position, position + forwardDir, math::vec3f(0.0f, 1.0f, 0.0f));
    }
        
    void Camera::updateViewMatrix()
    {
        viewMatrix = math::lookAt(position, position + forwardDir, math::vec3f(0.0f, 1.0f, 0.0f));
    }

    void Camera::updateProjMatrix()
    {
        projMatrix = math::createPerspectiveDepthOneZero(aspect, fov, znear, zfar);
    }

    void Camera::update(const float dt)
    {        
        forwardDir.x = -std::sin(math::radians(rotation.x)) * std::cos(math::radians(rotation.y));
        forwardDir.y = std::sin(math::radians(rotation.y));
        forwardDir.z = -std::cos(math::radians(rotation.x)) * std::cos(math::radians(rotation.y));
        forwardDir = math::normalize(forwardDir);

        if (input::isKeyPressed(Keycode::A)) {
            position += math::normalize(math::cross(forwardDir, math::vec3f(0.0f, 1.0f, 0.0f))) * cameraSpeed * dt;
        }
        if (input::isKeyPressed(Keycode::D)) {
            position -= math::normalize(math::cross(forwardDir, math::vec3f(math::vec3f(0.0f, 1.0f, 0.0f)))) * cameraSpeed * dt;
        }
        if (input::isKeyPressed(Keycode::W)) {
            position += forwardDir * cameraSpeed * dt;
        }
        if (input::isKeyPressed(Keycode::S)) {
            position -= forwardDir * cameraSpeed * dt;
        }

        updateViewMatrix();
    }

    void Camera::onEvent(Event &event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) {
            aspect = static_cast<float>(e.width) / static_cast<float>(e.height);
            updateProjMatrix();
            return false;
        });

        dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &e) {
            if (e.mousecode == rotationButton) {
                isRotateButtonPressed = true;
                lastMousePos = input::getMousePosition();
                glfwSetInputMode(Application::getWindow().getNativeWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent &e) {
            if (e.mousecode == rotationButton) {
                isRotateButtonPressed = false;
                lastMousePos = {};
                glfwSetInputMode(Application::getWindow().getNativeWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }

            return false;
        });

        dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent &e) {
            if (isRotateButtonPressed) {
                float xOffset = (e.xpos - lastMousePos.x) * sensitivity;
                float yOffset = (e.ypos - lastMousePos.y) * sensitivity;
                lastMousePos = math::vec2f(e.xpos, e.ypos);

                rotation.x += xOffset;
                rotation.y += yOffset;
                rotation.y = std::clamp(rotation.y, -89.0f, 89.0f);
            }

            return false;
        });

        dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &e) {
            fov += e.yOffset * sensitivity * sensitivity;
            fov = std::clamp(fov, 1.0f, 120.0f);
            updateProjMatrix();
            return false;
        });
    }
}