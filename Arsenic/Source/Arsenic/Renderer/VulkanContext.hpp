#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"
#include "Arsenic/Renderer/Instance.hpp"
#include "Arsenic/Renderer/PhysicalDevice.hpp"
#include "Arsenic/Renderer/Structure.hpp"
#include "vk_mem_alloc.hpp"

struct GLFWwindow;

namespace arsenic
{
    struct VulkanContext 
    {
        Instance instance;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        PhysicalDevice physicalDevice = {};
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;
        VkQueue computerQueue = VK_NULL_HANDLE;
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;

        VkCommandPool tempCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer tempCommandBuffer = VK_NULL_HANDLE;
   
        VkFormat findSupportedFormat(const std::initializer_list<VkFormat> &candidates, const VkImageTiling tiling, 
                    const VkFormatFeatureFlags features) const;
    };  

       
    VulkanContext createVulkanContext(GLFWwindow* pGLfWwindow, const std::string& appName);
    void destroyVulkanContext(VulkanContext &renderContext);     
}