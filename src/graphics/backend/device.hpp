#pragma once
#include <vma/vk_mem_alloc.h>

#include "physical_device.hpp"

namespace graphics::internal
{

class VulkanBackend;
/// Container class for a logical device.
class Device
{
public:
    Device(const PhysicalDevice *_pDevice) : pDevice(_pDevice) {}
    ~Device() = default;
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    Device&& operator=(const Device&&) = delete;

    const VkDevice &GetDevice() { return device; }
    VkQueue GraphicsQueueHandle() { return graphicsQueue; }
    VkQueue PresentQueueHandle() { return presentQueue; }

    /// @brief Creates a buffer and allocates memory for it
    /// @param size Buffer size in bytes
    /// @param usage Buffer usage flags
    /// @param properties Buffer memory properties
    /// @param buffer Location to store created buffer
    /// @param bufferMemory Location to store allocated memory
    void CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);

    /// @brief Creates an image and allocates memory for it
    /// @param width 
    /// @param height 
    /// @param depth
    /// @param format Type of data stored in the image
    /// @param tiling Tiling arrangement of the image data
    /// @param usage Usage flags for the image
    /// @param properties Image memory properties
    /// @param image Location to store created image
    /// @param imageMemory Location to store allocated memory
    void CreateImage(
        uint32_t width,
        uint32_t height,
        uint32_t depth, // For 3D images
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);
    
    // Utilities

    /// @brief Idle until the device is ready 
    inline void WaitIdle() { vkDeviceWaitIdle(device); }

    /// @brief Create a temporary command buffer
    /// @return Vulkan command buffer handle
    VkCommandBuffer BeginSingleTimeCommands();
    /// @brief End temporary command buffer
    /// @param commandBuffer Temporary command buffer to end
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
private:
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;

    VmaAllocator allocator;

    const PhysicalDevice *pDevice;
    void createLogicalDevice(const PhysicalDevice &physicalDevice, bool enableValidationLayers);
    void createCommandPool();
    
    void cleanup();

    friend class VulkanBackend;
};

} // namespace graphics::internal