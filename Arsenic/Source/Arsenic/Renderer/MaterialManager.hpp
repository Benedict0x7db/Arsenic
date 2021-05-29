#pragma once

#include "Arsenic/Math/Math.hpp"
#include "Arsenic/Renderer/Structure.hpp"
#include "Arsenic/Renderer/Handle.hpp"

namespace arsenic
{
    struct VulkanContext;
    class TextureManager;
    class BufferManager;

    class MaterialManager
    {
    public:
        MaterialManager() = default;
        ~MaterialManager() = default;

        MaterialManager(const MaterialManager &) = delete;    
        MaterialManager &operator=(const MaterialManager &) = delete;
        MaterialManager(MaterialManager &&) = delete;
        MaterialManager &operator=(MaterialManager &&) = delete;

        void initialize(const VulkanContext &renderContext);
        void deInitialize(const VulkanContext &renderContext);

        Material createMaterial(const float roughness, const float metalness, const math::vec3f baseColor, const math::vec3f emissiveColor = math::vec3f(0.0f));
        Material createMaterial();

        VkSampler createSampler(const VulkanContext &renderContext);
    private:
    };
}