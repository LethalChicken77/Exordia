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

    void CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryProperties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);

    void CreateImage(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        VkImageCreateInfo &createInfo,
        VkMemoryPropertyFlags memoryProperties,
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