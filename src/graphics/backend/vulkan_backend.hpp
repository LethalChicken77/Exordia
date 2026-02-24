#pragma once
#include <memory>
#include <string>
#include <vector>

#include "graphics/backend/vulkan_include.h"

#include "device.hpp"

namespace graphics{class Graphics;}
namespace graphics::internal
{
    
class VulkanBackend
{
    public:
        VulkanBackend() = default;
        ~VulkanBackend();

        // VkBuffer AllocateBuffer();
        // VkImage AllocateImage();

        Device &GetDevice() { return device; }
        const PhysicalDevice &GetPhysicalDevice() { return physicalDevice; }

        inline void WaitForDevice() { device.WaitIdle(); }
    private:
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
        void init(const std::string& appName, const std::string& engName, Window& window); // Initialize backend, must be called before use

        VkApplicationInfo appInfo{};
        
        VkInstance instance{};
        PhysicalDevice physicalDevice;
        Device device{physicalDevice};
        VkDebugUtilsMessengerEXT debugMessenger;

        // Point to centralized values
        const std::string* applicationName;
        const std::string* engineName;

        void createInstance();
        void createDevice();

        // Utilities
        void setupDebugMessenger();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        std::vector<const char *> getRequiredExtensions();
        bool hasGflwRequiredInstanceExtensions();
        bool checkValidationLayerSupport();

        friend class graphics::Graphics;
};
} // namespace graphics::internal