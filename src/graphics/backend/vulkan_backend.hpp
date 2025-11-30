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

        VkInstance instance{};
        PhysicalDevice physicalDevice;
        Device device;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        // Point to centralized values
        const std::string* applicationName;
        const std::string* engineName;

        void createInstance();
        void createDevice();
        void createSurface(GLFWwindow* window);

        // Utilities
        void setupDebugMessenger();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        std::vector<const char *> getRequiredExtensions();
        bool hasGflwRequiredInstanceExtensions();
        bool checkValidationLayerSupport();
};
} // namespace graphics::internal