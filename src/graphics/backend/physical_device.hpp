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
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::set<vk::PresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    const vk::PhysicalDevice &Get() const { return physicalDevice; }
    const vk::PhysicalDeviceProperties2 &GetProperties() const { return properties; }
    const vk::PhysicalDeviceLimits GetLimits() const { return properties.properties.limits; }
    const QueueFamilyIndices &GetQueueFamilyIndices() const { return queueFamilyIndices; }
    SwapchainSupportDetails &GetSwapchainSupportDetails(vk::SurfaceKHR* surface) { 
        swapchainSupport = querySwapChainSupport(surface);
        return swapchainSupport;
    }
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    vk::Format FindSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

    // uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    // VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
    
private:
    PhysicalDevice();

    vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;

    vk::PhysicalDeviceProperties2 properties{};

    QueueFamilyIndices queueFamilyIndices{};
    SwapchainSupportDetails swapchainSupport{};
    // VkPhysicalDeviceMemoryProperties memoryProperties{};

    void init(vk::Instance instance, vk::SurfaceKHR* surface);
    void pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR* surface);
    bool findDeviceCapabilities(vk::SurfaceKHR* surface);
    
    // Utilities
    bool checkDeviceExtensionSupport();
    QueueFamilyIndices findQueueFamilies(vk::SurfaceKHR* surface);
    SwapchainSupportDetails querySwapChainSupport(vk::SurfaceKHR* surface);

    friend class VulkanBackend;
};

} // namespace graphics