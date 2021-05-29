#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Core/Application.hpp"
#include "Arsenic/Core/Input.hpp"

#include <GLFW/glfw3.h>

namespace arsenic
{
    namespace input
    {
        bool isKeyPressed(const Keycode keycode)
        {
            auto window = static_cast<GLFWwindow*>(Application::getWindow().getNativeWindowHandle());
            const auto state = glfwGetKey(window, static_cast<int>(keycode));

            return state == GLFW_PRESS || GLFW_RELEASE;
        }

        bool isMouseButtonPressed(const Mousecode mousecode)
        {
            auto window = static_cast<GLFWwindow*>(Application::getWindow().getNativeWindowHandle());
            const auto state = glfwGetMouseButton(window, static_cast<int>(mousecode));

            return state == GLFW_PRESS;
        }

        bool isMouseButtonReleased(const Mousecode mousecode)
        {
            auto window = static_cast<GLFWwindow*>(Application::getWindow().getNativeWindowHandle());
            const auto state = glfwGetMouseButton(window, static_cast<int>(mousecode));
            return state == GLFW_RELEASE;
        }

        math::vec2f getMousePosition()
        {
            auto window = static_cast<GLFWwindow*>(Application::getWindow().getNativeWindowHandle());

            double xpos = 0.0;
            double ypos = 0.0f;
            glfwGetCursorPos(window, &xpos, &ypos);

            return math::vec2f(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    }
}