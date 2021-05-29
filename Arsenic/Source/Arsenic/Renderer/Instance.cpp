#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/VulkanHeader.hpp"
#include "Arsenic/Renderer/Instance.hpp"

#include <GLFW/glfw3.h>

namespace arsenic
{
#ifdef NDEBUG
    constexpr bool enableValidation = false;
#else
    constexpr bool enableValidation = true;
#endif
    static constexpr std::array<const char *, 1> layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    static bool checkInstanceLayersSupport()
    {
        uint32_t supportedLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);

        assert(supportedLayerCount != 0);

        std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
        vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

        for (const auto &layerToCheck : layers) {
            bool layerFound = false;

            for (const auto &layer : supportedLayers) {
                if (std::strcmp(layerToCheck, layer.layerName) == 0) {
                    layerFound = true;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static bool checkInstanceExtensions(const std::vector<const char *> &instanceExtensions)
    {
        uint32_t supportedExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);

        assert(supportedExtensionCount != 0);

        std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

        for (const auto &extensionToCheck : instanceExtensions) {
            bool extensionFound = false;

            for (const auto &extension : supportedExtensions) {
                if (std::strcmp(extensionToCheck, extension.extensionName) == 0) {
                    extensionFound = true;
                }
            }

            if (!extensionFound) {
                return false;
            }
        }

        return true;
    }

    static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocationCallback,
                                                 VkDebugUtilsMessengerEXT *pDebugUtilsMessengerEXT)
    {
        auto pfn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        assert(pfn != nullptr);

		return pfn(instance, pCreateInfo, pAllocationCallback, pDebugUtilsMessengerEXT);
    }

    static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks *pAllocationCallbacks)
    {
        auto pfn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        assert(pfn != nullptr);

        pfn(instance, VkDebugUtilsMessengerEXT(debugUtilsMessengerEXT), pAllocationCallbacks);
    }

    VKAPI_ATTR static VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData)
    {
        ARSENIC_WARN("Validation Layer:\t{}", pCallbackData->pMessage);
        return VK_FALSE;
    }

    static void createDebugUtilsMessenger(Instance &instance)
    {
        if constexpr (enableValidation) {
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
            debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

            debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsMessengerCI.pfnUserCallback = debugCallback;

            checkVkResult(createDebugUtilsMessengerEXT(instance.vkinstance, &debugUtilsMessengerCI, nullptr, &instance.debugMessenger));
        }
    }

    Instance Instance::createVulkanInstance(const std::string& appName)
    {    
        assert(checkInstanceLayersSupport());
                
        VkApplicationInfo applicationCI = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
        applicationCI.pApplicationName = appName.c_str();
        applicationCI.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationCI.pEngineName = "Arsenic";
        applicationCI.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationCI.apiVersion = VK_API_VERSION_1_1;

        constexpr uint32_t i = VK_API_VERSION_1_0;
        constexpr uint32_t k = VK_MAKE_VERSION(1, 0, 108);

        uint32_t glfwExtensionCount = 0;
        const char **ppGLFWExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        assert(glfwExtensionCount != 0);

        std::vector<const char *> instanceExtensions(ppGLFWExtensions, ppGLFWExtensions + glfwExtensionCount);
        
        assert(checkInstanceExtensions(instanceExtensions));

        VkInstanceCreateInfo instanceCI = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceCI.pApplicationInfo = &applicationCI;
        instanceCI.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instanceCI.ppEnabledExtensionNames = instanceExtensions.data();

        if constexpr (enableValidation) {
            instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            assert(checkInstanceExtensions(instanceExtensions));

            instanceCI.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
            instanceCI.ppEnabledExtensionNames = instanceExtensions.data();
            instanceCI.enabledLayerCount = static_cast<uint32_t>(layers.size());
            instanceCI.ppEnabledLayerNames = layers.data();
        }

        Instance instance = {};
        checkVkResult(vkCreateInstance(&instanceCI, nullptr, &instance.vkinstance));
        createDebugUtilsMessenger(instance);

        return instance;
    }

    void Instance::destroyVulkanInstance(Instance *pInstance)
    {
        if constexpr (enableValidation){
            destroyDebugUtilsMessengerEXT(pInstance->vkinstance, pInstance->debugMessenger, nullptr);
        }

        vkDestroyInstance(pInstance->vkinstance, nullptr);
    }
}