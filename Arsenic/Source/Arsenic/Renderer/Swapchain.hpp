#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"

namespace arsenic
{
    struct VulkanContext;

    struct Swapchain 
    {
        VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
        VkFormat imageFormat;
        VkExtent2D imageExtent = {};
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        uint32_t minImageCount = 0;
        uint32_t imageCount = 0;
    };

    Swapchain createSwapchain(const VkDevice device, const VkSurfaceKHR surface, const VkPhysicalDevice physicalDevice, const VkExtent2D desiredExtent);
    void destroySwapchain(const VkDevice device, Swapchain &swapchain);

    uint32_t acquireImageFromSwapchain(const VkDevice, const VkSemaphore imageReadySemaphore, const Swapchain &swapchain);
    void presentSwapchainImage(const VkDevice device, const VkQueue presentQueue, const VkSemaphore waitSemaphore, const uint32_t imageIndex, const Swapchain &swapchain);
}