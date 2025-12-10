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
    Device(PhysicalDevice &_pDevice);
    ~Device() = default;
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    Device&& operator=(const Device&&) = delete;

    VkQueue GraphicsQueueHandle() { return graphicsQueue; }
    VkQueue PresentQueueHandle() { return presentQueue; }
    
    // Utilities

    /// @brief Idle until the device is ready 
    inline void WaitIdle() { vkDeviceWaitIdle(device); }

    /// @brief Create a temporary command buffer
    /// @return Vulkan command buffer handle
    VkCommandBuffer BeginSingleTimeCommands();
    /// @brief End temporary command buffer
    /// @param commandBuffer Temporary command buffer to end
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    const VkDevice &GetDevice() const { return device; }
    PhysicalDevice &GetPhysicalDevice() { return pDevice; }
    const VmaAllocator &GetAllocator() const { return allocator; }
    const VkCommandPool &GetCommandPool() const { return commandPool; }
    const VkQueue &GetGraphicsQueue() const { return graphicsQueue; }
    const VkQueue &GetPresentQueue() const { return presentQueue; }
private:
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;

    VmaAllocator allocator;

    PhysicalDevice &pDevice;
    void createLogicalDevice(bool enableValidationLayers);
    void createCommandPool();
    void createAllocator(VkInstance &instance);
    
    void cleanup();

    friend class VulkanBackend;
};

} // namespace graphics::internal