#pragma once
#include "graphics/backend/vulkan_include.h"

#include "graphics/backend/device.hpp"
// #define DISABLE_VALIDATION // Disable validation checks for performance. Ensure correctness manually.

namespace graphics
{

class Image;
class Buffer
{
public:
    Buffer(
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        vk::BufferUsageFlags usageFlags,
        vk::MemoryPropertyFlags requiredMemoryProperties = {},
        vk::DeviceSize minOffsetAlignment = 1);
    Buffer(
        internal::Device &device,
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        vk::BufferUsageFlags usageFlags,
        vk::MemoryPropertyFlags requiredMemoryProperties = {},
        vk::DeviceSize minOffsetAlignment = 1);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&&);
    Buffer& operator=(Buffer&&) = delete;

    static Buffer CreateStagingBuffer(
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        bool isSource = true,
        vk::DeviceSize minOffsetAlignment = 1);

    static Buffer CreateStagingBuffer(
        internal::Device &device,
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        bool isSource = true,
        vk::DeviceSize minOffsetAlignment = 1);

    // void WriteDataStaged(void* data, size_t size); // TODO: Implement
    
    vk::Result Map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    void Unmap();

    void WriteData(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    void ReadData(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    void Flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::DescriptorBufferInfo DescriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) const noexcept;
    void Invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

    void WriteToIndex(void* data, int index, uint32_t count = 1);
    void ReadFromIndex(void* data, int index, uint32_t count = 1);
    void FlushIndex(int index, uint32_t count = 1);
    vk::DescriptorBufferInfo DescriptorInfoForIndex(int index, uint32_t count = 1) const noexcept;
    void InvalidateIndex(int index, uint32_t count = 1);

    void CopyFromBuffer(const Buffer &srcBuffer, vk::DeviceSize size);
    void CopyFromImage(const Image &srcImage, uint32_t width, uint32_t height, uint32_t layerCount);
    void CopyFromImage(const Image &srcImage, uint32_t width, uint32_t height, uint32_t layerCount, const vk::BufferImageCopy &region);

    vk::Buffer GetBuffer() const noexcept { return buffer; }
    void* GetDataStart() const noexcept { return bufferAllocationInfo.pMappedData; }
    uint32_t GetInstanceCount() const noexcept { return instanceCount; }
    vk::DeviceSize GetInstanceSize() const noexcept { return instanceSize; }
    vk::DeviceSize GetAlignmentSize() const noexcept { return alignmentSize; }
    vk::BufferUsageFlags GetUsageFlags() const noexcept { return usageFlags; }
    vk::MemoryPropertyFlags GetMemoryPropertyFlags() const {
        return device.GetAllocator().getAllocationMemoryProperties(bufferAllocation);
    }
    vk::DeviceSize GetSize() const { return bufferSize; }
    vk::DescriptorBufferInfo GetDescriptorInfo(size_t offset = 0, size_t range = VK_WHOLE_SIZE) const { return { buffer, offset, range }; }
private:
    internal::Device &device;

    vk::Buffer buffer = VK_NULL_HANDLE;
    vma::Allocation bufferAllocation = VK_NULL_HANDLE;
    vma::AllocationInfo bufferAllocationInfo;

    vk::DeviceSize bufferSize;
    uint32_t instanceCount;
    vk::DeviceSize instanceSize;
    vk::DeviceSize alignmentSize;
    
    vk::BufferUsageFlags usageFlags;
    
    static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

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

    friend class Image;
};

} // namespace graphics::internal