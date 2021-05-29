#pragma once

#include "Arsenic/Scene/Component.hpp"

#include <entt/entt.hpp>

namespace arsenic
{
    class Entity;

    class Scene
    {
    public:
        Scene() = default;

        Entity createEntity();
        void destroyEntity(Entity &entity);

        const entt::registry &getRegistry() const { return m_enttRegistry; }
        entt::registry &getRegistry() { return m_enttRegistry; }
    private:
        entt::registry m_enttRegistry;
        friend class Entity;
    };
}