#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Scene/Entity.hpp"
#include "Arsenic/Scene/Component.hpp"
#include "Arsenic/Scene/Scene.hpp"

namespace arsenic
{
    Entity Scene::createEntity()
    {
        Entity entity(m_enttRegistry.create(), this);   
        entity.addComponent<Transform>();
        return entity;
    }
    
    void Scene::destroyEntity(Entity &entity)
    {
        assert(this == entity.m_pScene);
        assert(entity.isValid());
        
        m_enttRegistry.remove_all(entity);
        m_enttRegistry.destroy(entity);   

        entity = {};
    }
}