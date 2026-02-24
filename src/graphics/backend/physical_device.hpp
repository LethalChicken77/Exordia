#pragma once
// Library headers
#include <set>
#include "graphics/backend/vulkan_include.h"
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
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::set<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    const VkPhysicalDevice &GetPhysicalDevice() const { return physicalDevice; }
    const VkPhysicalDeviceProperties2 &GetProperties() const { return properties; }
    const VkPhysicalDeviceDescriptorBufferPropertiesEXT &GetDescriptorBufferProperties() const { return descriptorBufferProperties; }
    const QueueFamilyIndices &GetQueueFamilyIndices() const { return queueFamilyIndices; }
    SwapchainSupportDetails &GetSwapchainSupportDetails(VkSurfaceKHR* surface) { 
        swapchainSupport = querySwapChainSupport(physicalDevice, surface);
        return swapchainSupport;
    }
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

    // uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    // VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
    
private:
    PhysicalDevice();

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties2 properties{};
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};

    QueueFamilyIndices queueFamilyIndices{};
    SwapchainSupportDetails swapchainSupport{};
    // VkPhysicalDeviceMemoryProperties memoryProperties{};

    void init(VkInstance instance, VkSurfaceKHR* surface);
    void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR* surface);
    bool findDeviceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR* surface);
    
    // Utilities
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR* surface);
    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR* surface);

    friend class VulkanBackend;
};

} // namespace graphics