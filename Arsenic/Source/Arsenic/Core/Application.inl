#include "Arsenic/Core/Application.hpp"

namespace arsenic
{
    template<typename T, typename ...Args>
    inline void Application::pushLayer(Args &&...args)
    {
        static_assert(std::is_base_of_v<Layer, T>, "T has to be deprived from Layer");
        _layers.emplace(_layers.begin() + _layerInserter, createUniquePtr<T>(std::forward<Args>(args)...));
    }
    
    template<typename T, typename ...Args>
    inline void Application::pushOverLayer(Args &&...args)
    {
        static_assert(std::is_base_of_v<Layer, T>, "T has to be deprived from Layer");
        _layers.emplace_back(createUniquePtr<T>(std::forward<Args>(args)...));
    }
};