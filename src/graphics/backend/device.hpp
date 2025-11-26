#pragma once
// Library headers
#include <vulkan/vulkan.h>
// Project headers
#include "window.hpp"

namespace graphics::internal
{

class PhysicalDevice
{
public:
    PhysicalDevice();
    ~PhysicalDevice();
    
private:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    void pickPhysicalDevice();
};

class Device
{
public:
    Device(graphics::Window &window);
    ~Device();

    VkDevice GetDevice() { return device; }
    VkSurfaceKHR GetSurface() { return surface; }
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