#include "descriptor_buffer.hpp"

namespace graphics
{

uint32_t SizeHelper(const std::vector<ShaderLayout> &layouts, uint32_t setCount)
{
    uint32_t totalSize = 0;
    for (const ShaderLayout &layout : layouts)
    {
        totalSize += layout.GetSize() * setCount;
    }
    return totalSize;
}

uint32_t SizeHelper(const std::vector<ShaderLayout> &layouts, const std::vector<uint32_t> &setCounts)
{
    uint32_t totalSize = 0;
    for (uint32_t i = 0; i < layouts.size(); i++)
    {
        totalSize += layouts[i].GetSize() * setCounts[i];
    }
    return totalSize;
}

/// @brief Create a descriptor buffer with a single layout
/// @param device Device to create buffer on
/// @param layout Shader layout
/// @param setCount Number of sets to allocate for each layout
DescriptorBuffer::DescriptorBuffer(internal::Device &_device, const ShaderLayout &layout, uint32_t setCount, VkMemoryPropertyFlags memoryFlags) 
    : buffer(
        _device,
        SizeHelper(layouts, setCount),
        1,
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | 
            VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | 
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        memoryFlags
    ) {}

/// @brief Create a descriptor buffer with uniform set counts
/// @param device Device to create buffer on
/// @param layouts List of shader layouts to allocator for
/// @param setCount Number of sets to allocate for each layout
DescriptorBuffer::DescriptorBuffer(internal::Device &_device, const std::vector<ShaderLayout> &layouts, uint32_t setCount, VkMemoryPropertyFlags memoryFlags) 
    : buffer(
        _device,
        SizeHelper(layouts, setCount),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        memoryFlags
    ) {}

/// @brief Create a descriptor buffer
/// @param device Device to create buffer on
/// @param layouts List of shader layouts to allocator for
/// @param setCounts List of number of sets to allocate for each layout
DescriptorBuffer::DescriptorBuffer(internal::Device &_device, const std::vector<ShaderLayout> &layouts, const std::vector<uint32_t> &setCounts, VkMemoryPropertyFlags memoryFlags) 
    : buffer(
        _device,
        SizeHelper(layouts, setCounts),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        memoryFlags
    ) {}

DescriptorBuffer::~DescriptorBuffer()
{
    
}
} // namespace graphics