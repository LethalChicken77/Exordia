#pragma once
// Library headers
#include <vulkan/vulkan.h>
// Project headers
#include "graphics/window.hpp"

namespace graphics::internal
{

class PhysicalDevice
{
public:
    PhysicalDevice();
    ~PhysicalDevice();

    const VkPhysicalDevice &GetPhysicalDevice() { return physicalDevice; }
    
private:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    void pickPhysicalDevice();
};

class Device
{
public:
    Device(graphics::Window &window);
    ~Device();

    const VkDevice &GetDevice() { return device; }
    VkQueue GraphicsQueueHandle() { return graphicsQueue; }
    VkQueue PresentQueueHandle() { return presentQueue; }
private:
    VkDevice device;
    VkSurfaceKHR surface; // 
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    const PhysicalDevice &physicalDevice;
    void createLogicalDevice();
    void createCommandPool();
    void createSurface();
};

} // namespace graphics