#pragma once
// Library headers
#include <vulkan/vulkan.h>
// Project headers
#include "graphics/window.hpp"

namespace graphics::internal
{

class VulkanBackend;
/// Container for a physical device.
/// Includes functions for initializing device information.
class PhysicalDevice
{
public:
    const VkPhysicalDevice &GetPhysicalDevice() { return physicalDevice; }
    
private:
    PhysicalDevice();
    ~PhysicalDevice();
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties;
    void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR* surface);
    
    // Utilities
    bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR* surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR* surface);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR* surface);

    friend class VulkanBackend;
};

} // namespace graphics