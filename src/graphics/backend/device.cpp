#define VMA_IMPLEMENTATION
#include "device.hpp"
#include <vma/vk_mem_alloc.h>

#include <set>
#include <stdexcept>

#include "backend_data.hpp"

#include "utils/debug.hpp"
#include "utils/console.hpp"

#define VULKAN_HPP_NO_INCLUDE
#include <vulkan/vulkan.hpp>

namespace graphics::internal
{

Device::Device(PhysicalDevice &_pDevice) : pDevice(_pDevice){}

void Device::cleanup()
{
    if(allocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(allocator);
    if(commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(device, commandPool, nullptr);
    if(device != VK_NULL_HANDLE)
        vkDestroyDevice(device, nullptr);
}

vk::CommandBuffer Device::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&allocInfo, &commandBuffer);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(&beginInfo);
    return commandBuffer;
}

void Device::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

// Private methods
void Device::createLogicalDevice(bool enableValidationLayers)
{
    Console::log("Creating logical device", "Device");
    // pDevice = &physicalDevice;
    const PhysicalDevice::QueueFamilyIndices &indices = pDevice.GetQueueFamilyIndices();
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // TODO: Properly check feature support and dynamically enable them
    // TODO: Add support for extension features
    
    // Enable the feature for image float32 atomics
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures = {};
    // atomicFloatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
    // atomicFloatFeatures.shaderImageFloat32Atomics = VK_TRUE; // Enable float32 atomics on images
    // atomicFloatFeatures.shaderImageFloat32AtomicAdd = VK_TRUE; // Enable float32 atomics on images

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext = &features.featureChain.get<vk::PhysicalDeviceFeatures2>();//&atomicFloatFeatures; // Add the atomic float features to the device create info

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

    if (pDevice.Get().createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess) 
    {
        throw std::runtime_error("Failed to create logical device");
    }

    graphicsQueue = device.getQueue(indices.graphicsFamily, 0);
    presentQueue = device.getQueue(indices.presentFamily, 0);
}

void Device::createCommandPool() 
{
    const PhysicalDevice::QueueFamilyIndices &indices = pDevice.GetQueueFamilyIndices();

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if(device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) 
    {
        throw std::runtime_error("Failed to create command pool");
    }
}

void Device::createAllocator(vk::Instance &instance)
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.instance = instance;
    allocatorInfo.physicalDevice = pDevice.Get();
    allocatorInfo.device = device;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create VMA allocator");
    }
}

void Device::vmaDebugHandler(void* pUserData, const char* format, va_list args)
{
    printf(format, args);
}
} // namespace graphics::internal