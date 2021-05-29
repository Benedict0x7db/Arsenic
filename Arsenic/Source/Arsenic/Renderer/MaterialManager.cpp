#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/VulkanContext.hpp"
#include "Arsenic/Renderer/MaterialManager.hpp"

namespace arsenic
{
    void MaterialManager::initialize(const VulkanContext &renderContext)
    {
    }

    void MaterialManager::deInitialize(const VulkanContext &renderContext)
    {        
    }

    Material MaterialManager::createMaterial(const float roughness, const float metalness, const math::vec3f baseColor, const math::vec3f emissiveColor)
    {
        Material material = {};
        material.baseColorMapIndex = -1;
        material.roughnessMapIndex = -1;
        material.metalicMapIndex = -1;
        material.emissiveMapIndex = -1;
        material.normalMapIndex = -1;
        material.baseColor = math::vec4f(baseColor, 0.0f);
        material.roughness = roughness;
        material.metalness = metalness;
        material.emissiveColor = math::vec4f(emissiveColor, 0.0f);
        material.samplerIndex = 0;

        return material;
    }

    Material MaterialManager::createMaterial()
    {
        Material material = {};
        material.baseColorMapIndex = -1;
        material.roughnessMapIndex = -1;
        material.metalicMapIndex = -1;
        material.emissiveMapIndex = -1;
        material.normalMapIndex = -1;
        material.baseColor = math::vec4f(1.0f);
        material.roughness = 1.0f;
        material.metalness = 1.0f;
        material.emissiveColor = math::vec4f(0.0f);
        material.samplerIndex = 0;

        return material;
    }

    VkSampler MaterialManager::createSampler(const VulkanContext &renderContext)
    {
        VkSamplerCreateInfo samplerCI = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.minFilter = VK_FILTER_NEAREST;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        
        VkSampler sampler = VK_NULL_HANDLE;
        checkVkResult(vkCreateSampler(renderContext.device, &samplerCI, nullptr, &sampler));

        return sampler;
    }

}