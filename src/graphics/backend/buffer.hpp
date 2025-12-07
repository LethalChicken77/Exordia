#pragma once
#include <vulkan/vulkan.h>

#include "graphics/graphics_data.hpp"
// #define DISABLE_VALIDATION // Disable validation checks for performance. Ensure correctness manually.

namespace graphics
{

class Buffer
{
public:
    Buffer(
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags requiredMemoryProperties = 0,
        VkDeviceSize minOffsetAlignment = 1);
    Buffer(
        internal::Device &device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags requiredMemoryProperties = 0,
        VkDeviceSize minOffsetAlignment = 1);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    
    VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void Unmap();

    void WriteData(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void ReadData(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void WriteToIndex(void* data, int index, uint32_t count = 1);
    void ReadFromIndex(void* data, int index, uint32_t count = 1);
    VkResult FlushIndex(int index, uint32_t count = 1);
    VkDescriptorBufferInfo DescriptorInfoForIndex(int index, uint32_t count = 1);
    VkResult InvalidateIndex(int index, uint32_t count = 1);

    VkBuffer GetBuffer() const { return buffer; }
    void* GetDataStart() const { return bufferAllocationInfo.pMappedData; }
    uint32_t GetInstanceCount() const { return instanceCount; }
    VkDeviceSize GetInstanceSize() const { return instanceSize; }
    VkDeviceSize GetAlignmentSize() const { return alignmentSize; }
    VkBufferUsageFlags GetUsageFlags() const { return usageFlags; }
    VkMemoryPropertyFlags GetMemoryPropertyFlags() const { VkMemoryPropertyFlags flags;
        vmaGetAllocationMemoryProperties(device.GetAllocator(), bufferAllocation, &flags); 
        return flags;
    }
    VkDeviceSize GetBufferSize() const { return bufferSize; }
private:
    internal::Device &device;

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation bufferAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo bufferAllocationInfo;

    VkDeviceSize bufferSize;
    uint32_t instanceCount;
    VkDeviceSize instanceSize;
    VkDeviceSize alignmentSize;
    
    VkBufferUsageFlags usageFlags;
    
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    // Helper functions

    inline bool isCreated() const { return buffer != VK_NULL_HANDLE && bufferAllocation != VK_NULL_HANDLE; }
    inline bool isMapped() const { return bufferAllocationInfo.pMappedData != nullptr; }
    inline bool isHostVisible() const { 
        VkMemoryPropertyFlags flags;
        vmaGetAllocationMemoryProperties(device.GetAllocator(), bufferAllocation, &flags);
        return (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; 
    }
    inline bool isHostCoherent() const { 
        VkMemoryPropertyFlags flags;
        vmaGetAllocationMemoryProperties(device.GetAllocator(), bufferAllocation, &flags);
        return (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; 
    }
};

} // namespace graphics::internal