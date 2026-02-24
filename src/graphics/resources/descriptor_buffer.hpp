#pragma once
#include <memory>
#include "graphics/backend/device.hpp"
#include "graphics/backend/buffer.hpp"
#include "shader_layout.hpp"
#include "descriptor_set.hpp"

namespace graphics
{
class DescriptorBuffer
{
public:
    DescriptorBuffer(internal::Device &_device, const DescriptorSetLayout *layout, uint32_t setCount, VkMemoryPropertyFlags memoryFlags = GetDynamicFlags());
    DescriptorBuffer(internal::Device &device, const std::vector<DescriptorSetLayout*> &layouts, uint32_t setCount, VkMemoryPropertyFlags memoryFlags = GetDynamicFlags());
    DescriptorBuffer(internal::Device &device, const std::vector<DescriptorSetLayout*> &layouts, const std::vector<uint32_t> &setCounts, VkMemoryPropertyFlags memoryFlags = GetDynamicFlags());
    ~DescriptorBuffer();

    void SetValues(uint32_t index, ShaderLayout layout, const void* data);
    void AssignLayout(uint32_t index);
    void AddLayout(const ShaderLayout &layout, uint32_t setCount);

    void RecreateBuffer();

    /// @brief Memory property flags for frequently updated descriptors
    static constexpr VkMemoryPropertyFlags GetDynamicFlags() { return 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
        VK_MEMORY_PROPERTY_HOST_CACHED_BIT; 
    };

    /// @brief Memory property flags for rarely updated descriptors
    static constexpr VkMemoryPropertyFlags GetStaticFlags() { return 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    };

    /// @brief Memory property flags for constant descriptors
    static constexpr VkMemoryPropertyFlags GetConstFlags() { return 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    };
private:
    Buffer buffer;

    uint32_t maxSets = 1024;
    uint32_t maxSize = 1000000000; // 1 GB

    std::vector<DescriptorSetLayout*> layouts{};
    std::vector<uint32_t> setCounts{};
};
}