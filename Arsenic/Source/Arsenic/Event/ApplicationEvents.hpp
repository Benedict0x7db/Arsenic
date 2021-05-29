#pragma once

#include "Arsenic/Event/Event.hpp"

#include <cstdint>

namespace arsenic
{
    struct WindowCloseEvent final : Event
    {
        EventType getDynamicEventType() const override { return EventType::WindowClose; }
        static EventType getStaticEventType() noexcept { return EventType::WindowClose; }
    };

    struct WindowResizeEvent final : Event
    {
        const uint32_t width;
        const uint32_t height;

        WindowResizeEvent(const uint32_t width, const uint32_t height) :
            width(width),
            height(height)
        {
        }
        
        EventType getDynamicEventType() const override { return EventType::WindowResize; }
        static EventType getStaticEventType() noexcept { return EventType::WindowResize; }
    };
}