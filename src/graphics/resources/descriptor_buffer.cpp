#include "descriptor_buffer.hpp"
#include "descriptor_set.hpp"
#include "graphics/utils.hpp"

#include "utils/console.hpp"

namespace graphics
{

uint32_t SizeHelper(internal::Device &_device, const std::vector<DescriptorSetLayout> *&layouts, uint32_t setCount)
{
    uint32_t totalSize = 0;
    for (const DescriptorSetLayout &layout : layouts)
    {
        uint32_t layoutSize = _device.GetDescriptorSetLayoutSize(layout.GetDescriptorSetLayout());
        uint32_t alignment = _device.GetPhysicalDevice().GetDescriptorBufferProperties().descriptorBufferOffsetAlignment;
        totalSize += AlignedSize(layoutSize, alignment) * setCount;
    }
    return totalSize;
}

uint32_t SizeHelper(internal::Device &_device, const std::vector<DescriptorSetLayout> *&layouts, const std::vector<uint32_t> &setCounts)
{
#ifndef DISABLE_VALIDATION
    if(layouts.size() != setCounts.size())
    {
        Console::error("Must have same number of layouts and setCounts when creating DescriptorBuffer", "DescriptorBuffer");
        throw std::runtime_error("Layouts and setCounts size mismatch in SizeHelper");
    }
#endif
    uint32_t totalSize = 0;
    for (uint32_t i = 0; i < layouts.size(); i++)
    {
        uint32_t layoutSize = _device.GetDescriptorSetLayoutSize(layouts[i].GetDescriptorSetLayout());
        uint32_t alignment = _device.GetPhysicalDevice().GetDescriptorBufferProperties().descriptorBufferOffsetAlignment;
        totalSize += AlignedSize(layoutSize, alignment) * setCounts[i];
    }
    return totalSize;
}

/// @brief Create a descriptor buffer with a single layout
/// @param device Device to create buffer on
/// @param layout Descriptor set layout
/// @param setCount Number of sets to allocate for each layout
DescriptorBuffer::DescriptorBuffer(internal::Device &_device, const DescriptorSetLayout &layout, uint32_t setCount, VkMemoryPropertyFlags memoryFlags) 
    : buffer(
        _device,
        SizeHelper(_device, layouts, setCount),
        1,
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | 
            VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | 
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        memoryFlags
    ) {}

/// @brief Create a descriptor buffer with uniform set counts
/// @param device Device to create buffer on
/// @param layouts List of descriptor set layouts to allocator for
/// @param setCount Number of sets to allocate for each layout
DescriptorBuffer::DescriptorBuffer(internal::Device &_device, const std::vector<DescriptorSetLayout> &layouts, uint32_t setCount, VkMemoryPropertyFlags memoryFlags) 
    : buffer(
        _device,
        SizeHelper(_device, layouts, setCount),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        memoryFlags
    ) {}

/// @brief Create a descriptor buffer
/// @param device Device to create buffer on
/// @param layouts List of descriptor set layouts to allocator for
/// @param setCounts List of number of sets to allocate for each layout
DescriptorBuffer::DescriptorBuffer(internal::Device &_device, const std::vector<DescriptorSetLayout> &layouts, const std::vector<uint32_t> &setCounts, VkMemoryPropertyFlags memoryFlags) 
    : buffer(
        _device,
        SizeHelper(_device, layouts, setCounts),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        memoryFlags
    ) {}

DescriptorBuffer::~DescriptorBuffer()
{
    
}
} // namespace graphics