#pragma once
#include <vulkan/vulkan.h>

#include "graphics/graphics_data.hpp"

namespace graphics::internal
{

class Buffer
{
public:
    Buffer(
      VkDeviceSize instanceSize,
      uint32_t instanceCount,
      VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags,
      VkDeviceSize minOffsetAlignment = 1);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
private:
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    VkDeviceSize bufferSize;
    uint32_t instanceCount;
    VkDeviceSize instanceSize;
};

} // namespace graphics::internal