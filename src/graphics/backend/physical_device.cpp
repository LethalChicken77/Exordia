#include "physical_device.hpp"

#include <stdexcept>
#include <set>
#include <format>
#include "graphics/backend/vulkan_include.h"

#include "backend_data.hpp"
#include "console.hpp"
#include "debug.hpp"

namespace graphics::internal
{

PhysicalDevice::PhysicalDevice(vk::Instance* _instance) : instance(_instance) {}

void PhysicalDevice::init(vk::Instance instance, vk::SurfaceKHR* surface)
{
    pickPhysicalDevice(instance);
    // vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
}

void PhysicalDevice::pickPhysicalDevice(vk::Instance instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) 
    {
        throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
    }
    Console::log(std::format("Device count: {}", deviceCount), "PhysicalDevice");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const VkPhysicalDevice &device : devices) 
    {
        physicalDevice = device;
        if (isDeviceAdequate()) 
        {
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) 
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    properties = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = nullptr
    };
    properties = physicalDevice.getProperties2();
    Console::log("Physical device: " + std::string(properties.properties.deviceName), "PhysicalDevice");
}

bool PhysicalDevice::isDeviceAdequate() 
{
    // Note: We're ignoring the present queue family to avoid a dependency on surfaces.
    // This shouldn't be an issue as almost any device that can render can present.
    queryGraphicsFamily();
    
    bool extensionsSupported = checkDeviceExtensionSupport();

    Features testFeatures = features;
    physicalDevice.getFeatures2(&features.featureChain.get<vk::PhysicalDeviceFeatures2>());

    // TODO: Deal with feature checks dynamically
    return extensionsSupported && testFeatures == features && graphicsFamily.IsValid() && presentFamily.IsValid(); // TODO: Throw if invalid features instead of disabling them
}

bool PhysicalDevice::checkDeviceExtensionSupport() 
{
    uint32_t extensionCount;

    std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties &extension : availableExtensions) 
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void PhysicalDevice::queryGraphicsFamily() 
{
    graphicsFamily = QueueFamily{};

    uint32_t queueFamilyCount = 0;
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    bool idealFound = false;
    presentFamily = QueueFamily{};
    int i = 0;
    for (const VkQueueFamilyProperties &queueFamily : queueFamilies) 
    {
        bool presentSupport = glfwGetPhysicalDevicePresentationSupport(*instance, physicalDevice, i);
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) 
        {
            graphicsFamily.index = i;
            presentFamily.index = i;
            idealFound = true;
        }
        else if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            graphicsFamily.index = i;
        }
        else if(presentSupport)
        {
            presentFamily.index = i;
        }

        if(idealFound)
        {
            break;
        }

        i++;
    }
}

SwapchainSupportDetails PhysicalDevice::QuerySwapChainSupport(vk::SurfaceKHR* surface) 
{
    SwapchainSupportDetails details{};
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);

    details.formats = physicalDevice.getSurfaceFormatsKHR(*surface);

    std::vector<vk::PresentModeKHR> tempModes = physicalDevice.getSurfacePresentModesKHR(*surface);
    for (uint32_t i = 0; i < tempModes.size(); i++) 
    {
        details.presentModes.insert(tempModes[i]);
    }
    return details;
}

uint32_t PhysicalDevice::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

vk::Format PhysicalDevice::FindSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
{
    for (vk::Format format : candidates) 
    {
        vk::FormatProperties props;
        physicalDevice.getFormatProperties(format, &props);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) 
        {
            return format;
        } 
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) 
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format!");
}

} // namespace graphics::internal