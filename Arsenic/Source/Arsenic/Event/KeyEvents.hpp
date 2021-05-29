#pragma once

#include "Arsenic/Core/Keycode.hpp"
#include "Arsenic/Event/Event.hpp"

namespace arsenic
{
    struct KeyPressedEvent : Event
    {
        const Keycode keycode;

        KeyPressedEvent(const Keycode keycode) :
            keycode(keycode)
        {    
        }

        EventType getDynamicEventType() const override { return EventType::KeyPressed; }
        static constexpr EventType getStaticEventType() noexcept { return EventType::KeyPressed; }
    };

    struct KeyReleasedEvent : Event
    {
        Keycode keycode;
        
        KeyReleasedEvent(const Keycode keycode) :
            keycode(keycode)
        {    
        }

        EventType getDynamicEventType() const override { return EventType::KeyReleased; }
        static constexpr EventType getStaticEventType() noexcept { return EventType::KeyReleased; }
    };
}