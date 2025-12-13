#include "descriptor_set.hpp"

#include "utils/console.hpp"
#include "utils/debug.hpp"

namespace graphics
{

/// @brief Add a pool size to the descriptor pool builder
/// @param type Type of the pool size
/// @param size Size of the pool
DescriptorPool::Builder &DescriptorPool::Builder::AddPoolSize(VkDescriptorType type, uint32_t size)
{
    poolSizes.push_back({type, size});
    return *this;
}

/// @brief Set the pool flags for the descriptor pool builder
/// @param flags 
DescriptorPool::Builder &DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    poolFlags = flags;
    return *this;
}

/// @brief Set the max number of descriptor sets the pool can allocate
/// @param newMax 
DescriptorPool::Builder &DescriptorPool::Builder::SetMaxSets(uint32_t newMax)
{
    maxSets = newMax;
    return *this;
}

/// @brief Construct the pool
/// @return Unique pointer to the created descriptor pool
std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build()
{
    return std::unique_ptr<DescriptorPool>(new DescriptorPool(device, poolSizes, poolFlags, maxSets));
}

DescriptorPool::DescriptorPool(internal::Device &_device, const std::vector<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags, uint32_t maxSets)
    : device(_device)
{
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.flags = flags;
    createInfo.maxSets = maxSets;
    createInfo.pNext = nullptr;

    VK_CHECK(vkCreateDescriptorPool(device.GetDevice(), &createInfo, nullptr, &pool), "Failed to create descriptor pool");
}

DescriptorPool::~DescriptorPool()
{
    if(pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device.GetDevice(), pool, nullptr);
}

} // namespace graphics