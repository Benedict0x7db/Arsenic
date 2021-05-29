#pragma once

#include "Arsenic/Math/Math.hpp"
#include "Arsenic/Core/Keycode.hpp"
#include "Arsenic/Core/Mousecode.hpp"

namespace arsenic
{
    namespace input
    {
        bool isKeyPressed(const Keycode keycode);

        bool isMouseButtonPressed(const Mousecode mousecode);
        bool isMouseButtonReleased(const Mousecode mousecode);
        
        math::vec2f getMousePosition();
    }
}