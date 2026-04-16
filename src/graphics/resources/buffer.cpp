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
    vk::DeviceSize _instanceSize,
    uint32_t _instanceCount,
    vk::BufferUsageFlags _usageFlags,
    vk::MemoryPropertyFlags requiredMemoryProperties,
    vk::DeviceSize _minOffsetAlignment
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
    vk::DeviceSize _instanceSize,
    uint32_t _instanceCount,
    vk::BufferUsageFlags _usageFlags,
    vk::MemoryPropertyFlags requiredMemoryProperties,
    vk::DeviceSize _minOffsetAlignment) : device(_device)
{
    instanceSize = _instanceSize;
    instanceCount = _instanceCount;
    alignmentSize = getAlignment(_instanceSize, _minOffsetAlignment);
    bufferSize = alignmentSize * _instanceCount;
    usageFlags = _usageFlags;
    // #ifdef DEBUG
    // Console::debugf("Creating buffer: instance size: {}, aligned size: {}, instance count: {}, total size: {}", _instanceSize, instanceSize, instanceCount, bufferSize, "Buffer");
    // #endif
    
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = bufferSize;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    vma::AllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = vma::MemoryUsage::eAuto;
    allocCreateInfo.requiredFlags = requiredMemoryProperties;
    if (requiredMemoryProperties & vk::MemoryPropertyFlagBits::eHostVisible) // Without this it crashes on a device local buffer
    {
        allocCreateInfo.flags = vma::AllocationCreateFlagBits::eHostAccessRandom;
    }

    VK_CHECK(device.GetAllocator().createBuffer( 
        &bufferInfo, 
        &allocCreateInfo, 
        &buffer, 
        &bufferAllocation, 
        &bufferAllocationInfo), "Failed to create buffer");
    bufferAllocationInfo = device.GetAllocator().getAllocationInfo(bufferAllocation);
    // Console::debugf("Created vk::Buffer {}", (void*)buffer);
}

Buffer::~Buffer() 
{
    // Console::log("Destroying buffer", "Buffer");
    #ifndef DISABLE_VALIDATION
    Unmap();
    if(bufferAllocation != VK_NULL_HANDLE)
        vmaDestroyBuffer(device.GetAllocator(), buffer, bufferAllocation);
    #else
    vmaDestroyBuffer(device.GetAllocator(), buffer, bufferAllocation);
    #endif
}

Buffer::Buffer(Buffer&& other) 
    : device(other.device),
    buffer(other.buffer),
    bufferAllocation(other.bufferAllocation),
    bufferAllocationInfo{},
    bufferSize(other.bufferSize),
    instanceCount(other.instanceCount),
    instanceSize(other.instanceSize),
    alignmentSize(other.alignmentSize),
    usageFlags(other.usageFlags)
{
    other.buffer = VK_NULL_HANDLE;
    other.bufferAllocation = VK_NULL_HANDLE;
    bufferAllocationInfo = device.GetAllocator().getAllocationInfo(bufferAllocation);
    bufferAllocationInfo.pMappedData = other.bufferAllocationInfo.pMappedData;
    other.bufferAllocationInfo = vma::AllocationInfo{};
    other.bufferSize = 0;
}

Buffer Buffer::CreateStagingBuffer(
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        bool isSource,
        vk::DeviceSize minOffsetAlignment)
{
    return CreateStagingBuffer(
        graphicsData->GetBackend().GetDevice(),
        instanceSize,
        instanceCount,
        isSource,
        minOffsetAlignment
    );
}

Buffer Buffer::CreateStagingBuffer(
        internal::Device &device,
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        bool isSource,
        vk::DeviceSize minOffsetAlignment)
{
    return Buffer(
        device,
        instanceSize,
        instanceCount,
        isSource ? vk::BufferUsageFlagBits::eTransferSrc : vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        minOffsetAlignment
    );
}

// void Buffer::WriteDataStaged(void* data, size_t size)
// {
//     Buffer stagingBuffer = Buffer::CreateStagingBuffer(
//         device,
//         positionSize,
//         vertexCount
//     );

//     stagingBuffer.Map();
//     stagingBuffer.WriteData((void *)transforms.data(), bufferSize);

//     std::unique_ptr<Buffer> instanceBuffer = std::make_unique<Buffer>(
//         device,
//         instanceSize,
//         instanceCount,
//         vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
//         vk::MemoryPropertyFlagBits::eDeviceLocal
//     );

//     instanceBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
// }

/// @brief Assigns a memory range of the buffer to CPU accessible memory
/// @param size Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
/// @param offset Offset in bytes from the beginning of the buffer
/// @return Status of the map call
vk::Result Buffer::Map(vk::DeviceSize size, vk::DeviceSize offset)
{
    if(bufferAllocationInfo.pMappedData)
    {
        Console::warn("Buffer is already mapped", "Buffer");
    }
    void* tempData = nullptr;
    VkResult result = vmaMapMemory(device.GetAllocator(), bufferAllocation, &tempData);
    if(result != VK_SUCCESS)
    {
        Console::error(std::format("Failed to map buffer memory: {}", Debug::VkResultToString(result)), "Buffer");
    }
    bufferAllocationInfo.pMappedData = tempData;
    return (vk::Result)result;
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

/// @brief Write data to the buffer. 
/// @note Assumes data contains as many bytes as the buffer if the default size parameter is used. Incorrect usage can cause garbage to be written to the buffer, or even a crash.
/// @param data Pointer to the data to write from
/// @param size Size of the data in bytes
/// @param offset Offset in the buffer to start writing to in bytes
void Buffer::WriteData(void* data, vk::DeviceSize size, vk::DeviceSize offset)
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
    if(size + offset > bufferSize && size != VK_WHOLE_SIZE) // Need to allow for full-size allocation
    {
        Console::error(std::format("Write size exceeds buffer bounds (size: {}, offset: {}, size + offset: {} buffer size: {})", size, offset, size + offset, instanceCount * instanceSize), "Buffer");
        return;
    }
    #endif

    if (size == VK_WHOLE_SIZE) // NOTE: This assumes data is as large as the buffer
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
void Buffer::ReadData(void* resultData, vk::DeviceSize size, vk::DeviceSize offset)
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
void Buffer::Flush(vk::DeviceSize size, vk::DeviceSize offset)
{    
    device.GetAllocator().flushAllocation(bufferAllocation, offset, size);
}

/// @brief Get the descriptor info for the buffer
/// @param size Size of the buffer in bytes (default: VK_WHOLE_SIZE)
/// @param offset Offset in bytes from beginning (default: 0)
/// @return Descriptor info struct
vk::DescriptorBufferInfo Buffer::DescriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) const noexcept
{
    return vk::DescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
}

/// @brief Invalidate a memory range of the buffer to make it visible to the host
/// @param size Size of the memory range to invalidate (default: VK_WHOLE_SIZE)
/// @param offset Offset in bytes from beginning (default: 0)
/// @return Result of the invalidate call
void Buffer::Invalidate(vk::DeviceSize size, vk::DeviceSize offset)
{
    device.GetAllocator().invalidateAllocation(bufferAllocation, offset, size);
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
void Buffer::FlushIndex(int index, uint32_t count)
{
    Flush(instanceSize * count, index * instanceSize);
}

/// @brief Flush a memory range of the buffer at a specific index. Macro for DescriptorInfo.
/// @param index Index of the instance to get descriptor for
/// @param count Number of instances to include in descriptor (default: 1)
/// @return Descriptor info struct
vk::DescriptorBufferInfo Buffer::DescriptorInfoForIndex(int index, uint32_t count) const noexcept
{
    return DescriptorInfo(instanceSize, index * instanceSize);
}

/// @brief Invalidate a memory range of the buffer at a specific index. Macro for Invalidate.
/// @param index Index of the instance to invalidate
/// @param count Number of instances to invalidate (default: 1)
/// @return Result of the invalidate call
void Buffer::InvalidateIndex(int index, uint32_t count)
{
    Invalidate(instanceSize, index * instanceSize);
}

/// @brief Copy the contents from another buffer into this buffer
/// @param srcBuffer 
/// @param size 
void Buffer::CopyFromBuffer(const Buffer &srcBuffer, vk::DeviceSize size) 
{
    if(&device != &srcBuffer.device)
    {
        Console::error("Cannot copy buffer as source buffer does not belong to the same vk::Device", "Buffer");
        return;
    }
    // TODO: More validation
    // TODO: Support offsets

    vk::CommandBuffer commandBuffer = device.BeginSingleTimeCommands();

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer.GetBuffer(), buffer, 1, &copyRegion);

    device.EndSingleTimeCommands(commandBuffer);
}

/// @brief Copy the contents from an image into this buffer
/// @param srcImage 
/// @param width 
/// @param height 
/// @param layerCount 
void Buffer::CopyFromImage(const Image &srcImage, uint32_t width, uint32_t height, uint32_t layerCount) 
{
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{width, height, 1};

    CopyFromImage(srcImage, width, height, layerCount, region);
}

/// @brief Copy the contents from an image into this buffer
/// @param srcImage 
/// @param width 
/// @param height 
/// @param layerCount 
/// @param region 
void Buffer::CopyFromImage(const Image &srcImage, uint32_t width, uint32_t height, uint32_t layerCount, const vk::BufferImageCopy &region) 
{
    if(&device != &srcImage.device)
    {
        Console::error("Cannot copy buffer as source buffer does not belong to the same vk::Device", "Buffer");
        return;
    }
    vk::CommandBuffer commandBuffer = device.BeginSingleTimeCommands();

    vkCmdCopyImageToBuffer(
        commandBuffer,
        srcImage.GetImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer,
        1,
        (VkBufferImageCopy *)&region);
    device.EndSingleTimeCommands(commandBuffer);
}

vk::DeviceSize Buffer::getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) 
{
    if (minOffsetAlignment > 0) 
    {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}
} // namespace graphics::internal