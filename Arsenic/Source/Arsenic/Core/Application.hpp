#pragma once

#include "Arsenic/Core/Window.hpp"
#include "Arsenic/Core/Layer.hpp"
#include "Arsenic/Core/Utils.hpp"

namespace arsenic
{
    struct Event;
    struct WindowCloseEvent;
    struct WindowResizeEvent;

    class Application
    {
    public:        
        Application(const std::string &appName);

        Application(const Application &) = delete;
        const Application& operator=(const Application &) = delete;
        Application(Application &&) = delete;
        const Application& operator=(Application &&) = delete;

        virtual ~Application();

        void run();

        template<typename T, typename ...Args>
        void pushLayer(Args &&...args);
        template<typename T, typename ...Args>
        void pushOverLayer(Args &&...args);

        static Window& getWindow() { return s_pInstance->_window; }
    private:   
        bool onWindowClose(WindowCloseEvent &e);
        bool onWindowResize(WindowResizeEvent &e);
        void onEvent(Event &e);
    private:
        Window _window;
        std::vector<UniquePtr<Layer>> _layers;
        uint32_t _layerInserter = 0;
        bool _running;
        
        static Application *s_pInstance;
    };

    extern UniquePtr<Application> createApplication();
}

#include "Application.inl"
