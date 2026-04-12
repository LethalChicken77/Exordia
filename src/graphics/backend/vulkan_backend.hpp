#pragma once
#include <memory>
#include <string>
#include <vector>

#include "graphics/backend/vulkan_include.h"

#include "device.hpp"

namespace graphics{class Graphics;}
namespace graphics{class GraphicsData;}
namespace graphics::internal
{
    
class VulkanBackend
{
    public:
        VulkanBackend() = default;
        ~VulkanBackend();

        void Cleanup(graphics::GraphicsData *_graphicsData);

        inline vk::Instance &GetInstance() { return instance; }
        inline Device &GetDevice() { return device; }
        inline const PhysicalDevice &GetPhysicalDevice() const { return physicalDevice; }

        inline void WaitForDevice() { device.WaitIdle(); }
    private:
    #ifndef DEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
        void init(const std::string& appName, const std::string& engName, Window& window); // Initialize backend, must be called before use

        vk::ApplicationInfo appInfo{};
        
        vk::Instance instance{};
        PhysicalDevice physicalDevice;
        Device device{physicalDevice};
        vk::DebugUtilsMessengerEXT debugMessenger;
        
        bool cleanedUp = false;

        // Point to centralized values
        const std::string* applicationName;
        const std::string* engineName;

        void createInstance();
        void createDevice();

        // Utilities
        void setupDebugMessenger();
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
        std::vector<const char *> getRequiredExtensions();
        bool hasGflwRequiredInstanceExtensions();
        bool checkValidationLayerSupport();

        friend class graphics::Graphics;
};
} // namespace graphics::internal