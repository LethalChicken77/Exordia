#pragma once
// Library headers
#include <set>
#include "graphics/backend/vulkan_include.h"
// Project headers
#include "graphics/window.hpp"

namespace graphics::internal
{

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::set<vk::PresentModeKHR> presentModes;
};

struct QueueFamily {
    uint32_t index = ~0u;
    bool IsValid() { return index != ~0u; }
    bool operator==(const QueueFamily other) const
    {
        return index == other.index;
    }
};

class VulkanBackend;
/// Container for a physical device.
/// Includes functions for initializing device information.
class PhysicalDevice
{
public:

    const vk::PhysicalDevice &Get() const { return physicalDevice; }
    const vk::PhysicalDeviceProperties2 &GetProperties() const { return properties; }
    const vk::PhysicalDeviceLimits GetLimits() const { return properties.properties.limits; }
    inline QueueFamily GetGraphicsFamily() const { return graphicsFamily; };
    inline QueueFamily GetPresentFamily() const { return presentFamily; };
    inline std::vector<QueueFamily> GetUniqueQueues() const 
    {
        if(graphicsFamily == presentFamily)
            return {graphicsFamily};
        else
            return {graphicsFamily, presentFamily};
    }

    SwapchainSupportDetails QuerySwapChainSupport(vk::SurfaceKHR* surface);


    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    vk::Format FindSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

    // uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    // VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
    
private:
    PhysicalDevice() = default;
    PhysicalDevice(vk::Instance* _instance);

    vk::Instance* instance = nullptr;
    vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
    vk::PhysicalDeviceProperties2 properties{};
    QueueFamily graphicsFamily{};
    QueueFamily presentFamily{};

    // VkPhysicalDeviceMemoryProperties memoryProperties{};
    void pickPhysicalDevice(vk::Instance instance);

    void init(vk::Instance instance, vk::SurfaceKHR* surface);

    /// @brief Check if the physical device supports the requested extensions and features.
    bool isDeviceAdequate();
    /// @brief Check if the requested extensions are supported.
    bool checkDeviceExtensionSupport();
    /// @brief Check if the graphics family is adequate.
    void queryGraphicsFamily();

    friend class VulkanBackend;
};

} // namespace graphics