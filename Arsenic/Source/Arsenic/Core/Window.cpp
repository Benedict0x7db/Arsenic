#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Core/Keycode.hpp"
#include "Arsenic/Event/Event.hpp"
#include "Arsenic/Event/ApplicationEvents.hpp"
#include "Arsenic/Event/KeyEvents.hpp"
#include "Arsenic/Event/MouseEvents.hpp"

#include "Arsenic/Core/Window.hpp"

namespace arsenic
{
    Window::Window(const std::string &appName) 
    {
        const auto status = glfwInit();
        assert(status);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_pGLFWwindow = glfwCreateWindow(1280, 720, appName.c_str(), nullptr, nullptr);
        assert(m_pGLFWwindow);
               
        glfwSetWindowUserPointer(m_pGLFWwindow, this);
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_pGLFWwindow);
        glfwTerminate();
    }

    void Window::onUpdate()
    {
        glfwPollEvents();
    }

    void Window::initEventCallbacks(std::function<void(Event&)> &&eventCallback)
    {
        m_eventCallback = std::move(eventCallback);

        glfwSetWindowCloseCallback(m_pGLFWwindow, [](GLFWwindow *pGLFWwindow) {
            auto *pWindowswindow = static_cast<Window*>(glfwGetWindowUserPointer(pGLFWwindow));

            WindowCloseEvent e;
            pWindowswindow->m_eventCallback(e);     
        });

        glfwSetWindowSizeCallback(m_pGLFWwindow, [](GLFWwindow *pGLFWwindow, int width, int height) {
            auto *pWindowswindow = static_cast<Window*>(glfwGetWindowUserPointer(pGLFWwindow));
            
            WindowResizeEvent e(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

            pWindowswindow->m_eventCallback(e);     
        });
        
        glfwSetKeyCallback(m_pGLFWwindow, [](GLFWwindow* pGLFWwindow, int key, int scancode, int action, int mod) {
            switch (auto* pWindowswindow = static_cast<Window*>(glfwGetWindowUserPointer(pGLFWwindow)); action) {
                case GLFW_PRESS: {
                    KeyPressedEvent e(static_cast<Keycode>(key));
                    pWindowswindow->m_eventCallback(e);
                    break;
                }
                
                case GLFW_RELEASE: {
                    KeyReleasedEvent e(static_cast<Keycode>(key));
                    pWindowswindow->m_eventCallback(e);
                    break;
                };
            };
        });

        glfwSetCursorPosCallback(m_pGLFWwindow, [](GLFWwindow *pGLFWwindow, double xpos, double ypos) {
            auto* pWindowswindow = static_cast<Window*>(glfwGetWindowUserPointer(pGLFWwindow));
            MouseMovedEvent e(static_cast<float>(xpos), static_cast<float>(ypos));
            pWindowswindow->m_eventCallback(e);
        });

        glfwSetScrollCallback(m_pGLFWwindow, [](GLFWwindow *pGLFWwindow, double xoffset, double yoffset) {
            auto* pWindowswindow = static_cast<Window*>(glfwGetWindowUserPointer(pGLFWwindow));
            MouseScrolledEvent e(static_cast<float>(xoffset), static_cast<float>(yoffset));
            pWindowswindow->m_eventCallback(e);
        });

        glfwSetMouseButtonCallback(m_pGLFWwindow, [](GLFWwindow *pGLFWwindow, int button, int action, int mode) {
            switch (auto* pWindowswindow = static_cast<Window*>(glfwGetWindowUserPointer(pGLFWwindow)); action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent e(static_cast<Mousecode>(button));
                    pWindowswindow->m_eventCallback(e);
                    break;
                }
                
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent e(static_cast<Mousecode>(button));
                    pWindowswindow->m_eventCallback(e);
                    break;
                };
            };
        });
    }                            
}