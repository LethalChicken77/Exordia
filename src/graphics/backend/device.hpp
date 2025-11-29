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
    Device(graphics::Window &window);
    ~Device();
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    Device&& operator=(const Device&&) = delete;

    const VkDevice &GetDevice() { return device; }
    VkQueue GraphicsQueueHandle() { return graphicsQueue; }
    VkQueue PresentQueueHandle() { return presentQueue; }
private:
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;

    VmaAllocator allocator;

    const PhysicalDevice *pDevice;
    void createLogicalDevice(const PhysicalDevice &physicalDevice, bool enableValidationLayers);
    void createCommandPool();

    friend class VulkanBackend;
};

} // namespace graphics::internal