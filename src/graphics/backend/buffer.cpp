#include "buffer.hpp"
#include "utils/console.hpp"

namespace graphics
{
/// @brief Create buffer using default device
/// @param instanceSize Size of each instance in the buffer
/// @param instanceCount Number of instances in the buffer
/// @param usageFlags Vulkan buffer usage flags
/// @param memoryPropertyFlags Vulkan memory property flags
/// @param minOffsetAlignment (Optional) Minimum offset alignment required by the device
Buffer::Buffer(
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment) : device(graphicsData->GetBackend().GetDevice())
{
    // Using 'this' to avoid confusion between parameter and member variable
    this->instanceSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = instanceSize * instanceCount;
    this->instanceCount = instanceCount;
    this->usageFlags = usageFlags;
    this->memoryPropertyFlags = memoryPropertyFlags;

    device.CreateBuffer(
        bufferSize,
        usageFlags,
        memoryPropertyFlags,
        buffer,
        bufferMemory
    );
}

/// @brief Create buffer using specified device
/// @param device Device to create buffer on
/// @param instanceSize Size of each instance in the buffer
/// @param instanceCount Number of instances in the buffer
/// @param usageFlags Vulkan buffer usage flags
/// @param memoryPropertyFlags Vulkan memory property flags
/// @param minOffsetAlignment (Optional) Minimum offset alignment required by the device
Buffer::Buffer(
    internal::Device &device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment) : device(device)
{
    this->instanceSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = instanceSize * instanceCount;
    this->instanceCount = instanceCount;
    this->usageFlags = usageFlags;
    this->memoryPropertyFlags = memoryPropertyFlags;

    device.CreateBuffer(
        bufferSize,
        usageFlags,
        memoryPropertyFlags,
        buffer,
        bufferMemory
    );
}

Buffer::~Buffer() 
{
    if(buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(device.GetDevice(), buffer, nullptr);
    if(bufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(device.GetDevice(), bufferMemory, nullptr);
}

VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    if(buffer == VK_NULL_HANDLE || bufferMemory == VK_NULL_HANDLE)
    {
        Console::error("Buffer must be created before mapping", "Buffer");
        return VK_ERROR_UNKNOWN;
    }
    if(data != nullptr)
    {
        Console::warn("Buffer is already mapped", "Buffer");
        return VK_ERROR_UNKNOWN;
    }
    if(!(memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
    {
        Console::error("Buffer memory is not host visible and cannot be mapped", "Buffer");
        return VK_ERROR_UNKNOWN;
    }
    return vkMapMemory(device.GetDevice(), bufferMemory, offset, size, 0, &data);
}

void Buffer::Unmap() 
{
    if (data) 
    {
        vkUnmapMemory(device.GetDevice(), bufferMemory);
        data = nullptr;
    }
}

VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) 
{
    if (minOffsetAlignment > 0) 
    {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}
} // namespace graphics::internal