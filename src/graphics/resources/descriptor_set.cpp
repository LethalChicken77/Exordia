#include "descriptor_set.hpp"

#include "graphics/graphics_data.hpp"

#include "utils/console.hpp"
#include "utils/debug.hpp"

#include <ranges>

namespace graphics
{

DescriptorSetLayout::Builder::Builder() : Builder(graphicsData->GetBackend().GetDevice()) {}

/// @brief Add a binding to the new descriptor set layout
/// @param binding Binding index
/// @param descriptorType Type of the descriptor (buffer, image, etc.)
/// @param stageFlags Shader stages that will access this binding
/// @param count Number of descriptors in the binding
/// @return Reference to the builder for chaining
DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::AddBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count)
{
    if(bindings.count(binding) > 0)
    {
        Console::error(std::format("Binding {} already in use in DescriptorSetLayout::Builder", binding), "[DescriptorSetLayout::Builder]");
        return *this;
    }
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

/// @brief Build the descriptor set layout
/// @return Unique pointer to the created descriptor set layout
std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const 
{
    return std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout(device, bindings));
}

DescriptorSetLayout::DescriptorSetLayout(internal::Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) 
    : device(device), 
    bindings(bindings)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};

    for(const VkDescriptorSetLayoutBinding &binding : bindings | std::views::values)
    {
        setLayoutBindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = setLayoutBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(
        device.GetDevice(),
        &createInfo,
        nullptr,
        &descriptorSetLayout), "Failed to create descriptor set layout");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    if(descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device.GetDevice(), descriptorSetLayout, nullptr);
    }
}

// /// @brief Add a pool size to the descriptor pool builder
// /// @param type Type of the pool size
// /// @param size Size of the pool
// DescriptorPool::Builder &DescriptorPool::Builder::AddPoolSize(VkDescriptorType type, uint32_t size)
// {
//     poolSizes.push_back({type, size});
//     return *this;
// }

// /// @brief Set the pool flags for the descriptor pool builder
// /// @param flags 
// DescriptorPool::Builder &DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
// {
//     poolFlags = flags;
//     return *this;
// }

// /// @brief Set the max number of descriptor sets the pool can allocate
// /// @param newMax 
// DescriptorPool::Builder &DescriptorPool::Builder::SetMaxSets(uint32_t newMax)
// {
//     maxSets = newMax;
//     return *this;
// }

// /// @brief Construct the pool
// /// @return Unique pointer to the created descriptor pool
// std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build()
// {
//     return std::unique_ptr<DescriptorPool>(new DescriptorPool(device, poolSizes, poolFlags, maxSets));
// }

// DescriptorPool::DescriptorPool(internal::Device &_device, const std::vector<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags, uint32_t maxSets)
//     : device(_device)
// {
//     VkDescriptorPoolCreateInfo createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//     createInfo.poolSizeCount = poolSizes.size();
//     createInfo.pPoolSizes = poolSizes.data();
//     createInfo.flags = flags;
//     createInfo.maxSets = maxSets;
//     createInfo.pNext = nullptr;

//     VK_CHECK(vkCreateDescriptorPool(device.GetDevice(), &createInfo, nullptr, &pool), "Failed to create descriptor pool");
// }

// DescriptorPool::~DescriptorPool()
// {
//     if(pool != VK_NULL_HANDLE)
//         vkDestroyDescriptorPool(device.GetDevice(), pool, nullptr);
// }

} // namespace graphics