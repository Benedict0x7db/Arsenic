#pragma once

#include "Arsenic/Core/Mousecode.hpp"
#include "Arsenic/Event/Event.hpp"
#include "Arsenic/Math/Math.hpp"

namespace arsenic
{
    struct MouseScrolledEvent : Event
    {
        const float xOffset;
        const float yOffset;

        MouseScrolledEvent(const float xOffset, const float yOffset) :
            xOffset(xOffset),
            yOffset(yOffset)
        {
        }

        EventType getDynamicEventType() const override { return EventType::MouseScrolled; }  
        static constexpr EventType getStaticEventType() { return EventType::MouseScrolled; }  
    };

    struct MouseMovedEvent : Event
    {
        const float xpos;
        const float ypos;

        MouseMovedEvent(const float xpos, const float ypos) :
            xpos(xpos),
            ypos(ypos)
        {
        }

        EventType getDynamicEventType() const override { return EventType::MouseMoved; }  
        static constexpr EventType getStaticEventType() { return EventType::MouseMoved; }  
    };

    struct MouseButtonPressedEvent : Event 
    {
        const Mousecode mousecode;

        MouseButtonPressedEvent(const Mousecode mousecode) :
            mousecode(mousecode)
        {
        }

        EventType getDynamicEventType() const override { return EventType::MouseButtonPressed; }  
        static constexpr EventType getStaticEventType() { return EventType::MouseButtonPressed; }  
    };

    struct MouseButtonReleasedEvent : Event 
    {
        const Mousecode mousecode;

        MouseButtonReleasedEvent(const Mousecode mousecode) :
            mousecode(mousecode)
        {
        }

        EventType getDynamicEventType() const override { return EventType::MouseButtonReleased; }  
        static constexpr EventType getStaticEventType() { return EventType::MouseButtonReleased; }  
    };
}