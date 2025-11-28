#include "physical_device.hpp"

#include <stdexcept>
#include <set>
#include <format>
#include <vulkan/vulkan.h>

#include "backend_data.hpp"
#include "utils/console.hpp"

namespace graphics::internal
{

PhysicalDevice::PhysicalDevice() {}

void PhysicalDevice::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR* surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
    }
    Console::log(std::format("Device count: %d", deviceCount), "VulkanBackend");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const VkPhysicalDevice &device : devices) 
    {
        if (isDeviceSuitable(device, surface)) 
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    Console::log("Physical device: " + std::string(properties.deviceName), "PhysicalDevice");
}

bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR* surface) 
{
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) 
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extensionCount,
        availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const VkExtensionProperties &extension : availableExtensions) 
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

PhysicalDevice::QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR* surface) 
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const VkQueueFamilyProperties &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

PhysicalDevice::SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR* surface) 
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, *surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, *surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, *surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            *surface,
            &presentModeCount,
            details.presentModes.data());
    }
    return details;
}

} // namespace graphics::internal