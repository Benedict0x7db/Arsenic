#pragma once

#include "Arsenic/Core/Utils.hpp"

#define ARSENIC_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace arsenic
{
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    class Event
    {
    public:
        Event() = default;
        virtual ~Event() = default;
        virtual EventType getDynamicEventType() const = 0;

        constexpr bool isEventHandled() const { return m_isHandled; }
    private:
        bool m_isHandled = false;

        friend class EventDispatcher;
    };

    class EventDispatcher 
    {
    public:
        EventDispatcher(Event &event) : m_event(event) {}
        
        EventDispatcher(const EventDispatcher &) = delete;
        EventDispatcher(EventDispatcher &&) = delete;
        
        template<typename EventListener, typename Function>
        void dispatch(Function &&callbackFunc)
        {
            if (EventListener::getStaticEventType() == m_event.getDynamicEventType()) {
                m_event.m_isHandled = callbackFunc(static_cast<EventListener&>(m_event));
            }
        }
    private:
        Event &m_event;
    };
}