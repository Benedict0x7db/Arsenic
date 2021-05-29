#pragma once

#include "Arsenic/Core/Logger.hpp"
#include "Arsenic/Scene/Scene.hpp"

#include <entt/entt.hpp>

namespace arsenic
{
    using EntityID = entt::entity;

    class Entity
    {
    public:
        Entity() = default;
        Entity(const Entity &) = default;

        Entity(const EntityID entityID, Scene *pScene) :
            m_entityID(entityID),
            m_pScene(pScene)
        {
            assert(pScene);
        }

        template<typename T, typename ...Args>
        T& addComponent(Args &&...args)
        {   
            assert(isValid());

            if (hasComponent<T>()) {
                return getComponent<T>();
            }

            return m_pScene->m_enttRegistry.emplace<T>(m_entityID, std::forward<Args>(args)...);
        }

        template<typename T>
        void removeComponent()
        {
            assert(isValid() && hasComponent<T>());
            m_pScene->m_enttRegistry.remove<T>(m_entityID);
        }         

        template<typename T>
        T& getComponent()
        {
            assert(isValid() && hasComponent<T>());
            return m_pScene->m_enttRegistry.get<T>(m_entityID);
        }
        
        template<typename T>
        const T& getComponent() const
        {
            assert(isValid() && hasComponent<T>());
            return m_pScene->m_enttRegistry.get<T>(m_entityID);
        }

        template<typename T>
        bool hasComponent() const
        {
            assert(isValid());
            return m_pScene->m_enttRegistry.any<T>(m_entityID);
        }

        bool isValid() const { return m_entityID != entt::null && m_pScene != nullptr; }
        operator entt::entity() const { return m_entityID; }
    private:
        EntityID m_entityID = entt::null;
        Scene *m_pScene = nullptr;

        friend class Scene;
    };
}