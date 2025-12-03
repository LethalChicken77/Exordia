#pragma once
#include <vulkan/vulkan.h>

#include "graphics/graphics_data.hpp"

namespace graphics
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
    Buffer(
        internal::Device &device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment = 1);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    
    VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void Unmap();

    void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void ReadFromBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void WriteToIndex(void* data, int index);
    VkResult FlushIndex(int index);
    VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
    VkResult InvalidateIndex(int index);

    VkBuffer GetBuffer() const { return buffer; }
    void* GetDataStart() const { return data; }
    uint32_t GetInstanceCount() const { return instanceCount; }
    VkDeviceSize GetInstanceSize() const { return instanceSize; }
    VkDeviceSize GetAlignmentSize() const { return instanceSize; }
    VkBufferUsageFlags GetUsageFlags() const { return usageFlags; }
    VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return memoryPropertyFlags; }
    VkDeviceSize GetBufferSize() const { return bufferSize; }
private:
    internal::Device &device;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    void *data = nullptr;

    VkDeviceSize bufferSize;
    uint32_t instanceCount;
    VkDeviceSize instanceSize;
    
    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;
    
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

};

} // namespace graphics::internal