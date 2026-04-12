#pragma once

#include "physical_device.hpp"
#ifdef DEBUG
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_LEAK_LOG_FORMAT(format, ...) do { \
        printf((format), __VA_ARGS__); \
        printf("\n"); \
    } while(false)
// #define VMA_DEBUG_LOG_FORMAT(format, ...) do { \
//     printf((format), __VA_ARGS__); \
//     printf("\n"); \
// } while(false)
#endif
#include <vma/vk_mem_alloc.h>

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

    vk::Queue GraphicsQueueHandle() { return graphicsQueue; }
    vk::Queue PresentQueueHandle() { return presentQueue; }
    
    // Utilities

    /// @brief Idle until the device is ready 
    inline void WaitIdle() { vkDeviceWaitIdle(device); }

    /// @brief Create a temporary command buffer
    /// @return Vulkan command buffer handle
    vk::CommandBuffer BeginSingleTimeCommands();
    /// @brief End temporary command buffer
    /// @param commandBuffer Temporary command buffer to end
    void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

    const vk::Device &Get() const { return device; }
    PhysicalDevice &GetPhysicalDevice() { return pDevice; }
    const VmaAllocator &GetAllocator() const { return allocator; }
    const vk::CommandPool &GetCommandPool() const { return commandPool; }
    const vk::Queue &GetGraphicsQueue() const { return graphicsQueue; }
    const vk::Queue &GetPresentQueue() const { return presentQueue; }

    uint32_t GetDescriptorSetLayoutSize(const vk::DescriptorSetLayout layout) const {
        VkDeviceSize layoutSize = 0;
        vkGetDescriptorSetLayoutSizeEXT(device, layout, &layoutSize);
        return layoutSize; // NOTE: Layout size should fit in uint32_t, if not you're doing something wrong
    }
private:
    vk::Device device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::CommandPool commandPool;

    VmaAllocator allocator;

    PhysicalDevice &pDevice;
    void createLogicalDevice(bool enableValidationLayers);
    void createCommandPool();
    void createAllocator(vk::Instance &instance);
    
    void cleanup();

    static void vmaDebugHandler(void* pUserData, const char* format, va_list args);

    friend class VulkanBackend;
};

} // namespace graphics::internal