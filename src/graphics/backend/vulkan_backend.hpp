#pragma once
#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "device.hpp"

namespace graphics::internal
{

class VulkanBackend
{
    public:
        VulkanBackend() = default;
        ~VulkanBackend();

        void Init(const std::string& appName, const std::string& engName, GLFWwindow* window); // Initialize backend, must be called before use

        const VkSurfaceKHR &GetSurface() { return surface; }
        // VkBuffer AllocateBuffer();
        // VkImage AllocateImage();
    private:
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Uncomment extensions to use them
            VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, 
            // VK_EXT_FILTER_CUBIC_EXTENSION_NAME, 
            // VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME 
        };

        struct VkInstanceRAII
        {
            VkInstance handle = VK_NULL_HANDLE;
            ~VkInstanceRAII()
            {
                if(handle) { vkDestroyInstance(handle, nullptr); }
            }
        };
        VkInstanceRAII instance{};
        PhysicalDevice physicalDevice;
        // Device device;
        std::unique_ptr<Device> device{}; // Why smart pointer?
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        // Point to centralized values
        const std::string* applicationName;
        const std::string* engineName;

        void createInstance();
        void initPhysicalDevice();
        void createDevice();
        void createSurface(GLFWwindow* window);

        // Utilities
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        std::vector<const char *> getRequiredExtensions();
        bool hasGflwRequiredInstanceExtensions();
        bool checkValidationLayerSupport();
};
} // namespace graphics::internal