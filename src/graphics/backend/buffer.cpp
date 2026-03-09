#include "buffer.hpp"

#include <format>

#include "image.hpp"
#include "graphics/graphics_data.hpp"

#include "utils/console.hpp"
#include "utils/debug.hpp"

namespace graphics
{
/// @brief Create buffer using default device
/// @param instanceSize Size of each instance in the buffer
/// @param instanceCount Number of instances in the buffer
/// @param usageFlags Vulkan buffer usage flags
/// @param minOffsetAlignment (Optional) Minimum offset alignment required by the device (default: 1)
Buffer::Buffer(
    VkDeviceSize _instanceSize,
    uint32_t _instanceCount,
    VkBufferUsageFlags _usageFlags,
    VkMemoryPropertyFlags requiredMemoryProperties,
    VkDeviceSize _minOffsetAlignment
) : Buffer(
        graphicsData->GetBackend().GetDevice(),
        _instanceSize,
        _instanceCount,
        _usageFlags,
        requiredMemoryProperties,
        _minOffsetAlignment) {}

/// @brief Create buffer using specified device
/// @param device Device to create buffer on
/// @param instanceSize Size of each instance in the buffer
/// @param instanceCount Number of instances in the buffer
/// @param usageFlags Vulkan buffer usage flags
/// @param minOffsetAlignment (Optional) Minimum offset alignment required by the device
Buffer::Buffer(
    internal::Device &_device,
    VkDeviceSize _instanceSize,
    uint32_t _instanceCount,
    VkBufferUsageFlags _usageFlags,
    VkMemoryPropertyFlags requiredMemoryProperties,
    VkDeviceSize _minOffsetAlignment) : device(_device)
{
    instanceSize = _instanceSize;
    instanceCount = _instanceCount;
    alignmentSize = getAlignment(_instanceSize, _minOffsetAlignment);
    bufferSize = alignmentSize * _instanceCount;
    usageFlags = _usageFlags;
    #ifdef DEBUG
    Console::debugf("Creating buffer: instance size: {}, aligned size: {}, instance count: {}, total size: {}", _instanceSize, instanceSize, instanceCount, bufferSize, "Buffer");
    #endif
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.requiredFlags = requiredMemoryProperties;
    if (requiredMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) // Without this it crashes on a device local buffer
    {
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    }

    VK_CHECK(vmaCreateBuffer(device.GetAllocator(), &bufferInfo, &allocCreateInfo, &buffer, &bufferAllocation, &bufferAllocationInfo), "Failed to create buffer");
    Console::debugf("Created VkBuffer {}", (void*)buffer);
}

Buffer::~Buffer() 
{
    Console::log("Destroying buffer", "Buffer");
    #ifndef DISABLE_VALIDATION
    Unmap();
    if(bufferAllocation != VK_NULL_HANDLE)
        vmaDestroyBuffer(device.GetAllocator(), buffer, bufferAllocation);
    #else
    vmaDestroyBuffer(device.GetAllocator(), buffer, bufferAllocation);
    #endif
}

/// @brief Assigns a memory range of the buffer to CPU accessible memory
/// @param size Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
/// @param offset Offset in bytes from the beginning of the buffer
/// @return Status of the map call
VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    void* tempData = nullptr;
    VkResult result = vmaMapMemory(device.GetAllocator(), bufferAllocation, &tempData);
    bufferAllocationInfo.pMappedData = tempData;
    #ifdef DEBUG
    if(result != VK_SUCCESS)
    {
        Console::error(std::format("Failed to map buffer memory: {}", Debug::VkResultToString(result)), "Buffer");
    }
    #endif
    return result;
}

/// @brief Unmaps a previously mapped memory range
void Buffer::Unmap() 
{
    if(bufferAllocationInfo.pMappedData) 
    {
        vmaUnmapMemory(device.GetAllocator(), bufferAllocation);
        bufferAllocationInfo.pMappedData = nullptr;
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
        memcpy(bufferAllocationInfo.pMappedData, data, bufferSize);
    } 
    else 
    {
        char *memOffset = (char *)bufferAllocationInfo.pMappedData;
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
        memcpy(resultData, bufferAllocationInfo.pMappedData, bufferSize);
    } 
    else 
    {
        char *memOffset = (char *)bufferAllocationInfo.pMappedData;
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
    return vmaFlushAllocation(device.GetAllocator(), bufferAllocation, offset, size);
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
    return vmaInvalidateAllocation(device.GetAllocator(), bufferAllocation, offset, size);
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

/// @brief Copy the contents from another buffer into this buffer
/// @param srcBuffer 
/// @param size 
void Buffer::CopyFromBuffer(const Buffer &srcBuffer, VkDeviceSize size) 
{
    if(&device != &srcBuffer.device)
    {
        Console::error("Cannot copy buffer as source buffer does not belong to the same VkDevice", "Buffer");
        return;
    }
    // TODO: More validation
    // TODO: Support offsets

    VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.GetBuffer(), buffer, 1, &copyRegion);

    device.EndSingleTimeCommands(commandBuffer);
}

/// @brief Copy the contents from an image into this buffer
/// @param srcImage 
/// @param width 
/// @param height 
/// @param layerCount 
void Buffer::CopyFromImage(const Image &srcImage, uint32_t width, uint32_t height, uint32_t layerCount) 
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    CopyFromImage(srcImage, width, height, layerCount, region);
}

/// @brief Copy the contents from an image into this buffer
/// @param srcImage 
/// @param width 
/// @param height 
/// @param layerCount 
/// @param region 
void Buffer::CopyFromImage(const Image &srcImage, uint32_t width, uint32_t height, uint32_t layerCount, const VkBufferImageCopy &region) 
{
    if(&device != &srcImage.device)
    {
        Console::error("Cannot copy buffer as source buffer does not belong to the same VkDevice", "Buffer");
        return;
    }
    VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

    vkCmdCopyImageToBuffer(
        commandBuffer,
        srcImage.GetImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer,
        1,
        &region);
    device.EndSingleTimeCommands(commandBuffer);
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