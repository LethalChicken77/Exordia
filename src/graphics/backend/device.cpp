#include <set>
#include <stdexcept>

#include "device.hpp"
#include "backend_data.hpp"

#include "utils/console.hpp"

namespace graphics::internal
{

void Device::cleanup()
{
    if(commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(device, commandPool, nullptr);
    if(device != VK_NULL_HANDLE)
        vkDestroyDevice(device, nullptr);
}

void Device::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory)
{
}

// Private methods
void Device::createLogicalDevice(const PhysicalDevice &physicalDevice, bool enableValidationLayers)
{
    Console::log("Creating logical device", "Device");
    pDevice = &physicalDevice;
    const PhysicalDevice::QueueFamilyIndices &indices = pDevice->GetQueueFamilyIndices();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Enable the feature for image float32 atomics
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures = {};
    // atomicFloatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
    // atomicFloatFeatures.shaderImageFloat32Atomics = VK_TRUE; // Enable float32 atomics on images
    // atomicFloatFeatures.shaderImageFloat32AtomicAdd = VK_TRUE; // Enable float32 atomics on images

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE; // Allow wireframe rendering

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext = nullptr;//&atomicFloatFeatures; // Add the atomic float features to the device create info

    // Might not really be necessary anymore because device specific validation layers
    // have been deprecated
    if(enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } 
    else 
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(pDevice->GetPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
}

void Device::createCommandPool() 
{
    const PhysicalDevice::QueueFamilyIndices &indices = pDevice->GetQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create command pool!");
    }
}
} // namespace graphics::internal