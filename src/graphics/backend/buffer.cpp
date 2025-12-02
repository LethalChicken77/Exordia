#include "buffer.hpp"

namespace graphics::internal
{
Buffer::Buffer(
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
{
    instanceSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = instanceSize * instanceCount;
    this->instanceCount = instanceCount;

    graphicsData->GetBackend().GetDevice().CreateBuffer(
        bufferSize,
        usageFlags,
        memoryPropertyFlags,
        buffer,
        bufferMemory
    );
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