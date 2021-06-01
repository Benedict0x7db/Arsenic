#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/Swapchain.hpp"

namespace arsenic
{
    struct SwapchainDetails
    {
        VkSurfaceCapabilitiesKHR surfaceCaps = {};
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    static SwapchainDetails querySwapchainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        SwapchainDetails swapchainChainDetails = {};

        checkVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainChainDetails.surfaceCaps));

        uint32_t surfaceFormatCount = 0;
        checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));

        swapchainChainDetails.surfaceFormats.resize(surfaceFormatCount);
        checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, swapchainChainDetails.surfaceFormats.data()));
        
        uint32_t presentModeCount = 0;
        checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));

        swapchainChainDetails.presentModes.resize(presentModeCount);
        checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapchainChainDetails.presentModes.data()));
        
        return std::move(swapchainChainDetails);
    }

    static VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR surfaceCaps, VkExtent2D desiredExtent)
    {
        if (surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return surfaceCaps.currentExtent;
        }

        desiredExtent.width = std::clamp(desiredExtent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        desiredExtent.height = std::clamp(desiredExtent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

        return desiredExtent;
    }

    static VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &presentModes)
    {
        for (const auto &presentMode : presentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    static VkSurfaceFormatKHR chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
    {
       for (const auto &surfaceFormat : surfaceFormats) {
            if (surfaceFormat.format == VK_FORMAT_R8G8B8_SNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return surfaceFormat;
            }
        }
        
        assert(surfaceFormats.size());
        return surfaceFormats[0];
    }

    Swapchain createSwapchain(const VkDevice device, const VkSurfaceKHR surface, const VkPhysicalDevice physicalDevice, const VkExtent2D desiredExtent)
    {
        const auto swapchainDetails = querySwapchainDetails(physicalDevice, surface);
        const auto swapchainExtent = chooseSwapchainExtent(swapchainDetails.surfaceCaps, desiredExtent);
        const auto swapchainPresentMpde = chooseSwapchainPresentMode(swapchainDetails.presentModes);
        const auto swapchainFormat = chooseSwapchainFormat(swapchainDetails.surfaceFormats);

        uint32_t imageCount = swapchainDetails.surfaceCaps.minImageCount + 1;

        if (swapchainDetails.surfaceCaps.maxImageCount != 0 && imageCount > swapchainDetails.surfaceCaps.maxImageCount) {
            imageCount = swapchainDetails.surfaceCaps.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainCI = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        swapchainCI.surface = surface;
        swapchainCI.imageExtent = swapchainExtent;
        swapchainCI.presentMode = swapchainPresentMpde;
        swapchainCI.imageFormat = swapchainFormat.format;
        swapchainCI.imageColorSpace = swapchainFormat.colorSpace;
        swapchainCI.minImageCount = imageCount;
        swapchainCI.imageArrayLayers = 1;
        swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.clipped = VK_TRUE;
        swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCI.oldSwapchain = VK_NULL_HANDLE;

        Swapchain swapchain = {};
        swapchain.imageFormat = swapchainFormat.format;
        swapchain.imageExtent = swapchainExtent;
        swapchain.minImageCount = swapchainCI.minImageCount;
        
        checkVkResult(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain.vkSwapchain));

        checkVkResult(vkGetSwapchainImagesKHR(device, swapchain.vkSwapchain, &swapchain.imageCount, nullptr));
        swapchain.images.resize(swapchain.imageCount);
        swapchain.imageViews.resize(swapchain.imageCount);
        checkVkResult(vkGetSwapchainImagesKHR(device, swapchain.vkSwapchain, &swapchain.imageCount, swapchain.images.data()));

        for (std::size_t i = 0; i != swapchain.imageCount; ++i) {
            VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            imageViewCI.image = swapchain.images[i];
            imageViewCI.format = static_cast<VkFormat>(swapchainCI.imageFormat);
            imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCI.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            checkVkResult(vkCreateImageView(device, &imageViewCI, nullptr, &swapchain.imageViews[i]));
        }
      
        return std::move(swapchain);
    }

    void destroySwapchain(VkDevice device, Swapchain &swapchain)
    {
        for (auto imageView : swapchain.imageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain.vkSwapchain, nullptr);
    } 

    uint32_t acquireImageFromSwapchain(const VkDevice device, const VkSemaphore imageReadySemaphore, const Swapchain &swapchain)
    {
        uint32_t imageIndex = 0;

        {
            VkResult result = vkAcquireNextImageKHR(device, swapchain.vkSwapchain, std::numeric_limits<uint64_t>::max(), imageReadySemaphore, VK_NULL_HANDLE, &imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                // TODO: error, but it works for now. you need to recheck image extent
                //recreateSwapchain(_swapchain.imageExtent);
            }
            else if (result != VK_SUCCESS) {
                ARSENIC_ERROR("Wait to acquire image from swapchain");
                assert(false);
            }
        }
        
        return imageIndex;
    }

    void presentSwapchainImage(const VkDevice device, const VkQueue presentQueue, const VkSemaphore waitSemaphore, const uint32_t imageIndex, const Swapchain &swapchain)
    {
        VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.vkSwapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;

        {
            VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                // TODO: error, but it works for now. you need to recheck image extent
                //recreateSwapchain(_swapchain.imageExtent);
            }
            else if (result != VK_SUCCESS) {
                ARSENIC_ERROR("Failed to present image");
                assert(false);
            }
        }

    }
}