#pragma once

using VkInstance = struct VkInstance_T*;
using VkDebugUtilsMessengerEXT = struct VkDebugUtilsMessengerEXT_T*;

namespace arsenic
{
    struct Instance
    {
        VkInstance vkinstance = {};
        VkDebugUtilsMessengerEXT debugMessenger = {};

        static Instance createVulkanInstance(const std::string& appName);
        static void destroyVulkanInstance(Instance *pInstance);
    };   
}