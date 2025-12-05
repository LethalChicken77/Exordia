#include "buffer.hpp"

#include <format>
#include "utils/console.hpp"
#include "utils/debug.hpp"

namespace graphics
{
/// @brief Create buffer using default device
/// @param instanceSize Size of each instance in the buffer
/// @param instanceCount Number of instances in the buffer
/// @param usageFlags Vulkan buffer usage flags
/// @param memoryPropertyFlags Vulkan memory property flags
/// @param minOffsetAlignment (Optional) Minimum offset alignment required by the device (default: 1)
Buffer::Buffer(
    VkDeviceSize _instanceSize,
    uint32_t _instanceCount,
    VkBufferUsageFlags _usageFlags,
    VkMemoryPropertyFlags _memoryPropertyFlags,
    VkDeviceSize _minOffsetAlignment
) : Buffer(
        graphicsData->GetBackend().GetDevice(),
        _instanceSize,
        _instanceCount,
        _usageFlags,
        _memoryPropertyFlags,
        _minOffsetAlignment) {}

/// @brief Create buffer using specified device
/// @param device Device to create buffer on
/// @param instanceSize Size of each instance in the buffer
/// @param instanceCount Number of instances in the buffer
/// @param usageFlags Vulkan buffer usage flags
/// @param memoryPropertyFlags Vulkan memory property flags
/// @param minOffsetAlignment (Optional) Minimum offset alignment required by the device
Buffer::Buffer(
    internal::Device &_device,
    VkDeviceSize _instanceSize,
    uint32_t _instanceCount,
    VkBufferUsageFlags _usageFlags,
    VkMemoryPropertyFlags _memoryPropertyFlags,
    VkDeviceSize _minOffsetAlignment) : device(_device)
{
    instanceSize = _instanceSize;
    instanceCount = _instanceCount;
    alignmentSize = getAlignment(_instanceSize, _minOffsetAlignment);
    bufferSize = alignmentSize * _instanceCount;
    usageFlags = _usageFlags;
    memoryPropertyFlags = _memoryPropertyFlags;
    #ifdef DEBUG
    Console::debug(std::format("Creating buffer: instance size: {}, aligned size: {}, instance count: {}, total size: {}", _instanceSize, instanceSize, instanceCount, bufferSize), "Buffer");
    #endif
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
    #ifndef DISABLE_VALIDATION
    if(buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(device.GetDevice(), buffer, nullptr);
    if(bufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(device.GetDevice(), bufferMemory, nullptr);
    #else
    vkDestroyBuffer(device.GetDevice(), buffer, nullptr);
    vkFreeMemory(device.GetDevice(), bufferMemory, nullptr);
    #endif
}

/// @brief Assigns a memory range of the buffer to CPU accessible memory
/// @param size Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
/// @param offset Offset in bytes from the beginning of the buffer
/// @return Status of the map call
VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    #ifndef DISABLE_VALIDATION
    if(!isCreated())
    {
        Console::error("Buffer must be created before mapping", "Buffer");
        return VK_ERROR_UNKNOWN;
    }
    if(isMapped())
    {
        Console::warn("Buffer is already mapped", "Buffer");
        return VK_ERROR_UNKNOWN;
    }
    if(!isHostVisible())
    {
        Console::error("Buffer memory is not host visible and cannot be mapped", "Buffer");
        return VK_ERROR_UNKNOWN;
    }
    #endif

    VkResult result = vkMapMemory(device.GetDevice(), bufferMemory, offset, size, 0, &bufferData);
    #ifdef DEBUG
    if(result != VK_SUCCESS)
    {
        Console::error(std::format("Failed to map buffer memory: ", Debug::VkResultToString(result)), "Buffer");
    }
    #endif
    return result;
}

/// @brief Unmaps a previously mapped memory range
void Buffer::Unmap() 
{
    if(bufferData) 
    {
        vkUnmapMemory(device.GetDevice(), bufferMemory);
        bufferData = nullptr;
    }
}

/// @brief Write data to the buffer
/// @param data Pointer to the data to write from
/// @param size Size of the data in bytes
/// @param offset Offset in the buffer to start writing to in bytes
void Buffer::WriteData(void* data, VkDeviceSize size, VkDeviceSize offset)
{
    #ifndef DISABLE_VALIDATION
    if(data == nullptr)
    {
        Console::error("Data pointer is null", "Buffer");
        return;
    }
    if(!isMapped())
    {
        Console::error("Buffer must be mapped before writing", "Buffer");
        return;
    }
    if(size + offset > bufferSize)
    {
        Console::error(std::format("Write size exceeds buffer bounds (size: {}, offset: {}, size + offset: {} buffer size: {})", size, offset, size + offset, instanceCount * instanceSize), "Buffer");
        return;
    }
    #endif

    if (size == VK_WHOLE_SIZE) 
    {
        memcpy(bufferData, data, bufferSize);
    } 
    else 
    {
        char *memOffset = (char *)bufferData;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/// @brief Read data from the buffer
/// @param data Pointer to the data to read into
/// @param size Size of the data in bytes
/// @param offset Offset in the buffer to start reading from in bytes
void Buffer::ReadData(void* resultData, VkDeviceSize size, VkDeviceSize offset)
{
    #ifndef DISABLE_VALIDATION
    if(resultData == nullptr)
    {
        Console::error("Data pointer is null", "Buffer");
        return;
    }
    if(!isMapped())
    {
        Console::error("Buffer must be mapped before reading", "Buffer");
        return;
    }
    if(size + offset > bufferSize)
    {
        Console::error(std::format("Write size exceeds buffer bounds (size: {}, offset: {}, size + offset: {} buffer size: {})", size, offset, size + offset, instanceCount * instanceSize), "Buffer");
        return;
    }
    #endif

    if (size == VK_WHOLE_SIZE) 
    {
        memcpy(resultData, bufferData, bufferSize);
    } 
    else 
    {
        char *memOffset = (char *)bufferData;
        memOffset += offset;
        memcpy(resultData, memOffset, size);
    }
}

/// @brief Flush a memory range of the buffer to make it visible to the device
/// @param size Size of the memory range to flush (default: VK_WHOLE_SIZE)
/// @param offset Offset in bytes from beginning (default: 0)
/// @return Status of the flush call
VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = bufferMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(device.GetDevice(), 1, &mappedRange);
}

/// @brief Get the descriptor info for the buffer
/// @param size Size of the buffer in bytes (default: VK_WHOLE_SIZE)
/// @param offset Offset in bytes from beginning (default: 0)
/// @return Descriptor info struct
VkDescriptorBufferInfo Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return VkDescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
}

/// @brief Invalidate a memory range of the buffer to make it visible to the host
/// @param size Size of the memory range to invalidate (default: VK_WHOLE_SIZE)
/// @param offset Offset in bytes from beginning (default: 0)
/// @return Result of the invalidate call
VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = bufferMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(device.GetDevice(), 1, &mappedRange);
}

/// @brief Write data to the buffer at a specific index. Macro for WriteData.
/// @param data Pointer to the data to write from
/// @param index Index of the instance to write to
/// @param count Size of input data in terms of instance size (default: 1)
void Buffer::WriteToIndex(void* data, int index, uint32_t count)
{
    WriteData(data, instanceSize * count, index * instanceSize);
}

/// @brief Read data from the buffer at a specific index. Macro for ReadData.
/// @param data Pointer to the data to read into
/// @param index Index of the instance to read from
/// @param count Size of output data in terms of instance size (default: 1)
void Buffer::ReadFromIndex(void* data, int index, uint32_t count)
{
    ReadData(data, instanceSize * count, index * instanceSize);
}

/// @brief Flush a memory range of the buffer at a specific index. Macro for Flush.
/// @param index Index of the instance to flush
/// @param count Number of instances to flush (default: 1)
VkResult Buffer::FlushIndex(int index, uint32_t count)
{
    return Flush(instanceSize * count, index * instanceSize);
}

/// @brief Flush a memory range of the buffer at a specific index. Macro for DescriptorInfo.
/// @param index Index of the instance to get descriptor for
/// @param count Number of instances to include in descriptor (default: 1)
/// @return Descriptor info struct
VkDescriptorBufferInfo Buffer::DescriptorInfoForIndex(int index, uint32_t count)
{
    return DescriptorInfo(instanceSize, index * instanceSize);
}

/// @brief Invalidate a memory range of the buffer at a specific index. Macro for Invalidate.
/// @param index Index of the instance to invalidate
/// @param count Number of instances to invalidate (default: 1)
/// @return Result of the invalidate call
VkResult Buffer::InvalidateIndex(int index, uint32_t count)
{
    return Invalidate(instanceSize, index * instanceSize);
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