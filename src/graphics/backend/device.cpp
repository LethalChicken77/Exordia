#define VMA_IMPLEMENTATION
#include "device.hpp"

#include "vk_mem_alloc.hpp"

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
    VK_CHECK(device.allocateCommandBuffers(&allocInfo, &commandBuffer), "Failed to allocate single time command buffer");

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    VK_CHECK(commandBuffer.begin(&beginInfo), "Failed to begin single time command buffer");
    return commandBuffer;
}

void Device::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_CHECK(graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE), "Failed to end single time command buffer");
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

// Private methods
void Device::createLogicalDevice(bool enableValidationLayers)
{
    Console::log("Creating logical device", "Device");
    // pDevice = &physicalDevice;
    
    // TODO: Invesigate whether to create a temporary surface to query the present family
    // const PhysicalDevice::QueueFamilyIndices &indices = pDevice.GetQueueFamilyIndices();
    // std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    // std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::vector<QueueFamily> uniqueQueueFamilies = pDevice.GetUniqueQueues();

    float queuePriority = 1.0f;
    for (QueueFamily queueFamily : uniqueQueueFamilies) 
    {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueFamilyIndex = queueFamily.index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    

    // TODO: Properly check feature support and dynamically enable them
    // TODO: Add support for extension features
    
    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext = &features.featureChain.get<vk::PhysicalDeviceFeatures2>();//&atomicFloatFeatures; // Add the atomic float features to the device create info

    if (pDevice.Get().createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess) 
    {
        throw std::runtime_error("Failed to create logical device");
    }

    graphicsQueue = device.getQueue(pDevice.GetGraphicsFamily().index, 0);
    presentQueue = device.getQueue(pDevice.GetPresentFamily().index, 0);
}

void Device::createCommandPool() 
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = pDevice.GetGraphicsFamily().index;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if(device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) 
    {
        throw std::runtime_error("Failed to create command pool");
    }
}

void Device::createAllocator(vk::Instance &instance)
{
    vma::VulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    vma::AllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.instance = instance;
    allocatorInfo.physicalDevice = pDevice.Get();
    allocatorInfo.device = device;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    VK_CHECK(vma::createAllocator(&allocatorInfo, &allocator), "Failed to create VMA allocator");
}

void Device::vmaDebugHandler(void* pUserData, const char* format, va_list args)
{
    printf(format, args);
}
} // namespace graphics::internal