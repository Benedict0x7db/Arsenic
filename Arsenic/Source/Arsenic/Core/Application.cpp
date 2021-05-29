#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Event/Event.hpp"
#include "Arsenic/Event/ApplicationEvents.hpp"
#include "Arsenic/Renderer/VulkanContext.hpp"
#include "Arsenic/Core/Layer.hpp"
#include "Arsenic/Core/Application.hpp"

namespace arsenic
{
    Application *Application::s_pInstance = nullptr;

    Application::Application(const std::string &appName) :
        _window(appName),
        _running(true)
    {
        assert(s_pInstance == nullptr);
        s_pInstance = this;

        _window.initEventCallbacks(ARSENIC_BIND_EVENT_FN(Application::onEvent));
    }

    Application::~Application()
    {
        for (auto &layer : _layers) {
            layer = nullptr;
        }
    }

    void Application::run()
    {
        float elaspedTime = 0.0f;

        while (_running) {
            const auto startFrame = std::chrono::steady_clock::now();

            for (auto &layer : _layers) {
                layer->onFrameBegin();
            }

            for (auto &layer : _layers) {
                layer->onUpdate(elaspedTime);
            }

            for (auto &layer : _layers) {
                layer->onRender();
            }

            for (auto &layer : _layers) {
                layer->onImGuiRender();
            }

            for (auto &layer : _layers) {
                layer->onFrameEnd();
            }

            _window.onUpdate();

            const auto endFrame = std::chrono::steady_clock::now();
            elaspedTime = std::chrono::duration<float>(endFrame - startFrame).count();
        }
    }
    
    bool Application::onWindowClose(WindowCloseEvent &e)
    {
        _running = false;
        return false;
    }
        
    bool Application::onWindowResize(WindowResizeEvent &e)
    {
        return false;
    }

    void Application::onEvent(Event &e)
    {        
        EventDispatcher dispatcher(e);

        dispatcher.dispatch<WindowCloseEvent>(ARSENIC_BIND_EVENT_FN(Application::onWindowClose));
        dispatcher.dispatch<WindowResizeEvent>(ARSENIC_BIND_EVENT_FN(Application::onWindowResize));

        for (auto layerIter = _layers.rbegin(); layerIter != _layers.rend(); ++layerIter) {
            if (!e.isEventHandled()) {
                (*layerIter)->onEvent(e);
            }
        }        
    }
}