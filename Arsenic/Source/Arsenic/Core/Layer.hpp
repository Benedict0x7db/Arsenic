#pragma once

#include "Arsenic/Event/Event.hpp"

namespace arsenic
{    
    struct Layer
    {
        Layer() = default;
        virtual ~Layer() = default;
    
        Layer(const Layer &) = delete;
        const Layer &operator=(const Layer &) = delete;
        Layer(Layer &&) = delete;
        const Layer &operator=(Layer &) = delete;

        virtual void onFrameBegin() {};
        virtual void onFrameEnd() {}
        virtual void onUpdate(const float dt) {}
        virtual void onRender() {}
        virtual void onImGuiRender() {}
        virtual void onEvent(Event &e) {}
    };
}