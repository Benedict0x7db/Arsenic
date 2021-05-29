#pragma once

#include <GLFW/glfw3.h>

namespace arsenic
{
    struct Event;

    class Window final
    {
    public:
        Window(const std::string &appName);

        Window(const Window &) = delete;
        Window(Window &&) = delete;

        ~Window();

        void onUpdate();

        void initEventCallbacks(std::function<void(Event&)> &&eventCallback);
        GLFWwindow* getNativeWindowHandle() const { return m_pGLFWwindow; }
    private:
        GLFWwindow *m_pGLFWwindow;
        std::function<void(Event&)> m_eventCallback;
    };
}