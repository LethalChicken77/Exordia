#pragma once
#include <vulkan/vulkan.h>

#include "graphics/graphics_data.hpp"
#define DEBUG // Enable debug logging
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

    void WriteData(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void ReadData(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void WriteToIndex(void* data, int index, uint32_t count = 1);
    VkResult FlushIndex(int index, uint32_t count = 1);
    VkDescriptorBufferInfo DescriptorInfoForIndex(int index, uint32_t count = 1);
    VkResult InvalidateIndex(int index, uint32_t count = 1);

    VkBuffer GetBuffer() const { return buffer; }
    void* GetDataStart() const { return bufferData; }
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
    void *bufferData = nullptr;

    VkDeviceSize bufferSize;
    uint32_t instanceCount;
    VkDeviceSize instanceSize;
    
    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;
    
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    // Helper functions

    inline bool isCreated() const { return buffer != VK_NULL_HANDLE && bufferMemory != VK_NULL_HANDLE; }
    inline bool isMapped() const { return bufferData != nullptr; }
    inline bool isHostVisible() const { return (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
    inline bool isHostCoherent() const { return (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0; }
};

} // namespace graphics::internal