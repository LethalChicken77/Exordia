#pragma once
#include "physical_device.hpp"

namespace graphics::internal
{

/// Container class for a logical device.
class Device
{
public:
    Device(graphics::Window &window, const PhysicalDevice &physicalDevice);
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
    // VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    const PhysicalDevice &physicalDevice;
    void createLogicalDevice();
    void createCommandPool();
    void createSurface();
};

} // namespace graphics::internal